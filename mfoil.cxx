
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
    fhChannel(0),	
	fhChannelPos(0),	
	fCurrents(0),
	fNumSparks(0),
    fSatVoltage(0),
    fFlagIsProcessed(kFALSE),
    fFlagIsLoaded(kFALSE),
    fFlagQuality(0),
	fQualityText(0)
{
    // default constructor
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
void MFoil::LoadFoilCurrents()
{	
	fFlagIsProcessed = kFALSE;
	// clear histogram array before loading, if loaded already
	if (fFlagIsLoaded) {
		std::cout << "clearing fhChannel\n";
		for (Int_t ich = 0; ich<fNumChannels; ich++) delete fhChannel.At(ich);
		fhChannel.Clear(); 
	}
    Int_t ndata = 0;
    std::ifstream inFile;
    inFile.open(fInFileName);
	std::string line;

	TString dat;
    std::vector<TString> date;
	
	std::string tim;
    std::vector<Double_t> times, times_shift, current_temp;
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
	// get measurement area:
	// hist minimum < x < current changes to 0
//	std::vector<Double_t> min, max;
//	for (Int_t ich = 0; ich < fNumChannels; ich++)
//	{
//		min.push_back(((TH1D*)fhChannel.At(ich))->GetMinimum());
//	}
	//Double_t min, max;
	//min = ((TH1D*)fhChannel.At(0))->GetMinimum();
	//max = ((TH1D*)fhChannel.At(0))->GetMaximum();
	//std::cout << min << "\t" << max << std::endl;
	
	// Determine saturation current and count
	// the number of sparks for each channel
	TSpectrum * s = new TSpectrum(10); // expecting less than 10 sparks

	TF1 * fconst = new TF1("fconst","pol0",0,((TH1D*)fhChannel.At(0))->GetXaxis()->GetXmax());
	fconst->SetParameter(0,0.0);
	Double_t threshold = 0;
	Int_t nsparks;
	for (Int_t ich = 0; ich < fNumChannels; ich++)
	{
		threshold = ((TH1D*)fhChannel.At(ich))->GetMaximum()/20.;
		nsparks = s->Search(((TH1D*)fhChannel.At(ich)),1,"",threshold);
		fNumSparks.push_back( nsparks );
		Double_t *xpeaks = s->GetPositionX();
		
		((TH1D*)fhChannel.At(ich))->Fit("fconst","qn");
		fSatVoltage.push_back( fconst->GetParameter(0) );
	}
	cout << "Found sparks = \n";
	for (Int_t ich = 0; ich < fNumChannels; ich++) cout << fNumSparks.at(ich) << endl;
	// draw measurement area (lines)

	// count sparks
	// spark definition: ???% change in ???seconds (few bins)

	// get saturation current:
	// fitting constant to 
	
	// update canvas : 
	// set background according to quality, show sparks
	fFlagIsProcessed = kTRUE;
}
//---------------------------------------------------------------------------------- 
int MFoil::GetNC() { return fNumChannels; }
//----------------------------------------------------------------------------------
int MFoil::GetType() { return fType; }
//----------------------------------------------------------------------------------
Bool_t MFoil::GetProcessedStatus() { return fFlagIsProcessed; }


#endif
