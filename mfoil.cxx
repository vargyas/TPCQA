
//#include "mutils.cxx"
#include "mfoil.h"

#ifndef MFOIL_CXX
#define MFOIL_CXX

const char *filetypes[] = { "All files",     "*",
                            "Text files",    "*.[tT][xX][tT]",
                            "ROOT files",    "*.root",
                            0,               0 };

//----------------------------------------------------------------------------------
Double_t String2Sec(const std::string timestring)
{
	const char * _ctime = timestring.c_str();
	std::istringstream _ss(_ctime);
	std::string _token;
	double _times_temp;
	double _times[3];
	int _itimes = 0;

	while (std::getline(_ss, _token, ':'))
	{
		std::istringstream(_token) >> _times_temp;
		_times[_itimes++] = _times_temp;
	}
	return _times[0] * 60 * 60 + _times[1] * 60 + _times[2];
}
//----------------------------------------------------------------------------------
MFoil::MFoil() :
	fType(0),
	fNumChannels(24),
    fName(0),
    fInFileName(0),
	fFlagIsProcessed(kFALSE),
	fFlagIsLoaded(kFALSE),
	fFlagQuality(0),
	fQualityText(0),
	fMeasurementStart(0),
	fMeasurementEnd(0),
	fSatCurrent(0),
	fNumSparks(0),
	fhChannel(0),
	fhChannelPos(0),
	fCurrents(0)
{
    // default constructor
	CreateHLimit();
}
//----------------------------------------------------------------------------------
MFoil::~MFoil() 
{
    // destructor
}
//----------------------------------------------------------------------------------
void MFoil::SetFileName(const TString filename)
{
    fInFileName = filename;
	// determine type (and number of channels):
	if (fInFileName.Contains("IROC")) {
		fType = 0;
		fNumChannels = 18;
	}
	if (fInFileName.Contains("OROC")) {
		fType = 1;
		fNumChannels = 24;
	}
	else {
		fType = 2;
		fNumChannels = 24;
	}
}
//----------------------------------------------------------------------------------
void MFoil::LoadFoilCurrents(const TString filename)
{	
	// setting the file name
	SetFileName(filename);

	// not yet processed, setting flag
	fFlagIsProcessed = kFALSE;
	
	// clear histogram array before loading, if loaded already
	if (fFlagIsLoaded) {
		std::cout << "clearing fhChannel\n";
		for (Int_t ich = 0; ich<fNumChannels; ich++) {
			delete fhChannel.At(ich);
			delete fhChannelPos.At(ich);
		}
		fhChannel.Clear(); 
		fhChannelPos.Clear(); 
	}
    Int_t ndata = 0;
    std::ifstream inFile;
    inFile.open(fInFileName);
	std::string line;

	TString dat;
    std::vector<TString> date;
	
	std::string tim;
    std::vector<Double_t> times, times_shift, current_temp;
	times.reserve(1000); times_shift.reserve(1000); current_temp.reserve(1000);
	date.reserve(1000);
	
    Double_t c[24];

	// open file and load currents
	while ( !inFile.eof() )
	{
		inFile >> dat >> tim;
		for (Int_t ich = 0; ich < fNumChannels; ich++) { inFile >> c[ich]; }

		date.push_back(dat);

		if(kFALSE)
		{
			//times.push_back(String2Sec(tim));
			// check eof this way, line.empty() is true for every second line... (?)
			if (ndata > 1) {
				if (times[ndata] == times[ndata - 1]) {
					break;
				}
			}
			//times_shift.push_back(times[ndata] - times[0]);
		}
		times_shift.push_back(ndata/2.);
		
		for (Int_t ich = 0; ich < fNumChannels; ich++) { current_temp.push_back(c[ich]); }

		fCurrents.push_back(current_temp);
		current_temp.clear();

		++ndata;
	} 
	// fill histogram array
	for (Int_t ich = 0; ich<fNumChannels; ich++)
	{
		fhChannel.Add(new TH1D(Form("h_ch%d", ich+1), Form("CH %d", ich+1), ndata+1, -0.25, double(ndata)/2. + 0.25));
		fhChannelPos.Add(new TH1D(Form("h_pos_ch%d", ich+1), Form("CH %d", ich+1), ndata+1, -0.25, double(ndata)/2. + 0.25));

        for(Int_t it=0; it<ndata; it++)
        {
            ((TH1D*)fhChannel.At(ich))->SetBinContent(it+1, fCurrents[it][ich]);
			((TH1D*)fhChannel.At(ich))->SetBinError(it+1, 1E-10);
			//((TH1D*)fhChannelPos.At(ich))->SetBinContent(it+1, TMath::Abs(fCurrents[it][ich]));
			((TH1D*)fhChannelPos.At(ich))->SetBinContent(it+1, -1.*(fCurrents[it][ich]));
			((TH1D*)fhChannelPos.At(ich))->SetBinError(it+1, 1E-10);
        }
    }
    fFlagIsLoaded = kTRUE;
}
//----------------------------------------------------------------------------------
void MFoil::ProcessFoilCurrents()
{
	fNumSparks.clear();
	
	// get measurement range:
	fMeasurementStart = DetectMeasurementStart() + 100; // avoid possible remnants from ramping
	fMeasurementEnd = DetectMeasurementStop();
	
	// add spark counting, and remove those before fitting
	// TODO
	// spark definition: ???% change in ???seconds (few bins)
	
	// determine saturation current by fitting constant
	TF1 * fconst = new TF1("fconst","pol0",fMeasurementStart, fMeasurementEnd);
	fconst->SetParameter(0,0.0);
	
	for (Int_t ich = 0; ich < fNumChannels; ich++)
	{
		((TH1D*)fhChannelPos.At(ich))->Fit(fconst, "RQ");
		fSatCurrent.push_back( fconst->GetParameter(0) );
	}

	fFlagIsProcessed = kTRUE;
}
//---------------------------------------------------------------------------------- 
Double_t GetMax(Double_t * array, int narray)
{
	Double_t xmax = array[0];
	for(int i=1; i<narray; i++)
	{
		if(array[i]>xmax) xmax=array[i];
	}
	return xmax;
}
//---------------------------------------------------------------------------------- 
Double_t MFoil::DetectMeasurementStart()
{
	TSpectrum * s = new TSpectrum(10); // expecting less than 10 ramps
	Double_t * xpeaks;
	Double_t xpeaks_max[24];
	Double_t threshold = 0;
	Int_t nramps;
	for (Int_t ich = 0; ich < fNumChannels; ich++)
	{
		threshold = ((TH1D*)fhChannelPos.At(ich))->GetMaximum()/5.;
		nramps = s->Search(((TH1D*)fhChannelPos.At(ich)),1,"nobackground",threshold); // don't automatically remove background 

		xpeaks_max[ich] = s->GetPositionX()[0];
	}
	return GetMax(xpeaks_max, fNumChannels);
}
//---------------------------------------------------------------------------------- 
Double_t MFoil::DetectMeasurementStop()
{
	// count consequitive bins with positive current
	// (negative here because of the flipped histogram)
	Int_t count=0;
	Int_t required_time=20; 
	Int_t lastbin = 1;
	Int_t n = ((TH1D*)fhChannelPos.At(0))->GetNbinsX();
	Double_t xmax[24];
	
	for (Int_t ich = 0; ich < fNumChannels; ich++)
	{
		TH1D * h = ((TH1D*)fhChannelPos.At(ich));
		count=0;
		for(int ib=1; ib<=n; ib++)
		{
			if(h->GetBinContent(ib) < 0) ++count;
			else count = 0;
			
			if(count > required_time) {
				lastbin = ib;
				break;
			}
		}
		lastbin = lastbin - required_time;
		xmax[ich] = h->GetBinCenter(lastbin);
	}
	return GetMax(xmax, fNumChannels);;
}
//---------------------------------------------------------------------------------- 
int MFoil::GetNC() { return fNumChannels; }
//----------------------------------------------------------------------------------
int MFoil::GetType() { return fType; }
//----------------------------------------------------------------------------------
Bool_t MFoil::GetProcessedStatus() const { return fFlagIsProcessed; }
//----------------------------------------------------------------------------------
Bool_t MFoil::GetLoadedStatus() const { return fFlagIsLoaded; }
//----------------------------------------------------------------------------------
TString MFoil::GetInfoSatCurrent(Int_t foil_id) const
{
	return Form("V_{sat} = %.3f nA\n",fSatCurrent[foil_id]*1E9);
}
//----------------------------------------------------------------------------------
TString MFoil::GetInfoNumSparks(Int_t foil_id) const
{
	return Form("# of sparks = %d", fNumSparks[foil_id]);	
}
//----------------------------------------------------------------------------------
TString MFoil::GetInFileName() const { return fInFileName; }
//----------------------------------------------------------------------------------
void MFoil::CreateHLimit()
{
	fHLimit = new TH1D("hLimit", "", 1, 0, 1E7);
	fHLimit->SetBinContent(1, 5E-10); // limit is 0.5 nA
}
//----------------------------------------------------------------------------------
void MFoil::DrawHLimit()
{	
	fHLimit->SetFillColor(kGreen+1);
	fHLimit->SetFillStyle(3003);
	fHLimit->SetLineColor(kGreen+1);
	fHLimit->Draw("afh same");
}
void  MFoil::DrawSatCurrent(Int_t ich)
{
	TLine * lcurrent = new TLine(fMeasurementStart, fSatCurrent.at(ich),fMeasurementEnd,  fSatCurrent.at(ich));
	lcurrent->SetLineColor(kBlack);
	lcurrent->SetLineWidth(3);
	
	lcurrent->Draw();
}
void  MFoil::DrawMeasurementRange(Int_t ich)
{
	TLine * lrangeLow = new TLine(fMeasurementStart, fSatCurrent.at(ich)/100., fMeasurementStart, fSatCurrent.at(ich)*100.);
	TLine * lrangeHigh= new TLine(fMeasurementEnd, fSatCurrent.at(ich)/100., fMeasurementEnd, fSatCurrent.at(ich)*100.);
	lrangeLow->SetLineWidth(2);
	lrangeHigh->SetLineWidth(2);
	
	lrangeLow->Draw();
	lrangeHigh->Draw();
}
//----------------------------------------------------------------------------------
void MFoil::DrawHChannel(Int_t ich, TString opt)
{
    ((TH1D*)fhChannelPos.At(ich))->SetLineColor(kBlue);
    fhChannelPos.At(ich)->Draw(opt);
}

#endif
