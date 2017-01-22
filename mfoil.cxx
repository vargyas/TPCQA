#include "mfoil.h"

#ifndef MFOIL_CXX
#define MFOIL_CXX


//----------------------------------------------------------------------------------
MFoil::MFoil() :
    fType(0),
    fNumChannels(24),
    fName(0),
    fInFileName(0),
    fFlagIsProcessed(kFALSE),
    fFlagIsLoaded(kFALSE),
    fFlagQuality(0),
    fnSparks(0),
    fQualityText(0),
    fMeasurementStart(0),
    fMeasurementEnd(0),
    fSatCurrent(0),
    fhSparks(0),
    fhChStdDev(0),
    fhChannel(0),
    fhChannelPos(0),
    fCurrents(0)
{
    // default constructor
	fhChannel.SetOwner(kTRUE);
	fhChannelPos.SetOwner(kTRUE);
	fhChStdDev.SetOwner(kTRUE);
	fhSparks.SetOwner(kTRUE);

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
    else if (fInFileName.Contains("OROC1")) {
        fType = 1;
        fNumChannels = 20;
    }
    else if (fInFileName.Contains("OROC2")) {
        fType = 2;
        fNumChannels = 22;
    }
    else if (fInFileName.Contains("OROC3")) {
        fType = 3;
        fNumChannels = 24;
    }
    else {
        std::cerr << "Foil type not recognized!\n";
        fType = 4;
        fNumChannels = 18;
    }
}
//----------------------------------------------------------------------------------
void MFoil::LoadFoilCurrents(const TString filename)
{   
    std::cout << "LoadFoilCurrents()\n";

    // setting the file name
    SetFileName(filename);

    // not yet processed, setting flag
    fFlagIsProcessed = kFALSE;
    
    // clear histogram array before loading, if loaded already
    if (fFlagIsLoaded) {
        std::cout << "clearing fhChannel\n";
		//for (Int_t ich = 0; ich<fNumChannels; ich++)
		//{
		//	delete fhChannel.At(ich);
		//	delete fhChannelPos.At(ich);
		//	delete fhChStdDev.At(ich);
		//	delete fhSparks.At(ich);
		//}
        fhChannel.Clear(); 
        fhChannelPos.Clear(); 
        fhChStdDev.Clear();
        fhSparks.Clear();
    }
    Int_t ndata = 0;
    std::ifstream inFile;
    inFile.open(fInFileName);
	std::string title;

    TString dat;
    std::vector<TString> date;
    
	std::string tim[3]; // dummy time (not used at the moment)
	Double_t time_shift;
    Double_t time_start;
	Double_t foilV, foilI;
	std::vector<Double_t> times_shift, current_temp;
	times_shift.reserve(1000); current_temp.reserve(24);
    date.reserve(1000);
    
    Double_t c[24];

	// open file and load currents with the GSI dataformat:
	// Date/C : Time : TimeStamp/D : time : VMeas : IMeas : I_00:I_01:I_02:I_03:I_04:I_05:I_06:I_07:I_08:I_09:I_10:I_11:I_12:I_13:I_14:I_15:I_16:I_17:I_18:I_19:I_20:I_21:I_22:I_23

	// read that title
	getline(inFile, title);

    while ( !inFile.eof() )
	{
		inFile >> tim[0] >> tim[1] >> time_shift >> tim[2] >> foilV >> foilI;
		for (Int_t ich = 0; ich < 24; ich++) { inFile >> c[ich]; } // file contains 24 values regardless of actual channels
		date.push_back( dat );
		times_shift.push_back(time_shift);
        if(ndata==0) time_start = times_shift[0]; 
		times_shift[ndata]-=time_start;

		for (Int_t ich = 0; ich < fNumChannels; ich++) { current_temp.push_back(c[ich]); }

		fCurrents.push_back(current_temp);
		current_temp.clear();

		++ndata;
	}

    // fill histogram array
    // discard last data if not even (histogram has ndata+1 bins...)
    if((ndata+1)%2!=0) { cout << "DISCARD LAST DATA\n"; ndata = ndata-1;}

    Double_t width = (times_shift[1]-times_shift[0])/2.;
    Double_t xmin  = *min_element(std::begin(times_shift), std::end(times_shift)) - width;
    Double_t xmax  = *max_element(std::begin(times_shift), std::end(times_shift)) + width;

    for (Int_t ich = 0; ich<fNumChannels; ich++)
    {
		fhChannel.Add( new TH1D(Form("h_ch%d", ich+1), Form("CH %d", ich+1), ndata+1, xmin, xmax) );
		fhChannelPos.Add( new TH1D(Form("h_pos_ch%d", ich+1), Form("CH %d", ich+1), ndata+1, xmin, xmax) );
		fhSparks.Add( new TH1D(Form("h_spark%d", ich+1), Form("CH %d", ich+1), ndata+1, xmin, xmax) );
        
        for(Int_t it=0; it<ndata; it++)
        {
            ((TH1D*)fhChannel.At(ich))->SetBinContent(it+1, fCurrents[it][ich]);
            ((TH1D*)fhChannel.At(ich))->SetBinError(it+1, TMath::Sqrt(TMath::Abs(fCurrents[it][ich])));
            ((TH1D*)fhChannelPos.At(ich))->SetBinContent(it+1, -1.*(fCurrents[it][ich]));
            ((TH1D*)fhChannelPos.At(ich))->SetBinError(it+1, TMath::Sqrt(TMath::Abs(fCurrents[it][ich])));
        }

        // create and fill std dev. histogram
        Double_t val = 0;
        Double_t median = GetMedian( (TH1D*)fhChannel.At(ich) );
        fhChStdDev.Add(new TH1D(Form("h_stddev%d", ich+1), Form("CH %d", ich+1), ndata+1, -3.*TMath::Abs(median), 3.*TMath::Abs(median)));
        for(Int_t it=0; it<ndata; it++)
        {
            val = ((TH1D*)fhChannel.At(ich))->GetBinContent(it+1);
            ((TH1D*)fhChStdDev.At(ich))->Fill( (median-val) );
        }
    }
    fFlagIsLoaded = kTRUE;
}
//----------------------------------------------------------------------------------
void MFoil::ProcessFoilCurrents()
{   
    if(fFlagIsProcessed)
    {
        fSatCurrent.clear();
        fFlagQuality.clear();
    }

    // get measurement range:
    fMeasurementEnd   = DetectMeasurementStop();
    fMeasurementStart = 0; // will be reset after saturation current estimation
    
    // estimating saturation current
    for (Int_t ich = 0; ich < fNumChannels; ++ich)
    {
        fSatCurrent.push_back( GetMedian( (TH1D*)fhChannel.At(ich) ) );
        // rebin to reduce fluctuations:
        //((TH1D*)fhChannelPos.At(ich))->Rebin(2);
        //((TH1D*)fhChannelPos.At(ich))->Scale(1./2.);
    }

    // detect sparks/ramps
    DetectSparks(fMeasurementEnd);
    
    // reset measurement start to happen after the last spark
    fMeasurementStart = DetectMeasurementStart();

    // correct estimated measurement range: 
    // get last spark of all channels
    for (Int_t ich = 0; ich < fNumChannels; ++ich)
    {
        Double_t max_spark = GetLastSparkPosition(ich);
        if(max_spark > fMeasurementStart)
            fMeasurementStart = max_spark;
    }
    std::cout << "Measurement range: "<<fMeasurementStart << "-" << fMeasurementEnd << " s" << std::endl;
    
    // calculate saturation current by averaging the determined and corrected measurement area
    Double_t satcurrent;
    for (Int_t ich = 0; ich < fNumChannels; ++ich)
    {
        Int_t istart  = ((TH1D*)fhChannel.At(ich))->FindBin(fMeasurementStart);
        Int_t istop   = ((TH1D*)fhChannel.At(ich))->FindBin(fMeasurementEnd);
        Int_t count   = 0;
        Double_t yval = 0;
        Double_t current = 0;

        for(Int_t ib=istart; ib<istop; ++ib)
        {
            yval = ((TH1D*)fhChannel.At(ich))->GetBinContent(ib);
            if(yval>0)
            {
                ++count;
                current+=yval;
            }
        }
        satcurrent = current/Double_t(count);
        fSatCurrent.at(ich) = satcurrent;
        
		if(satcurrent>=0.5) fFlagQuality.push_back(0); // bad foil
		else if(satcurrent<0.5 && satcurrent>0) fFlagQuality.push_back(1); // good foil
        else  fFlagQuality.push_back(2); // strange foil
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
// write own peak detector instead of TSpectrum
Double_t MFoil::DetectMeasurementStart()
{ 
    return GetLastSparkPosition(0); 
}
//----------------------------------------------------------------------------------
Double_t MFoil::EstimateSatCurrent(Int_t foil_id)
{
    // currently just the average of the positive bins
    Int_t npos = 0;
    Int_t n    = ((TH1D*)fhChannelPos.At(foil_id))->GetNbinsX();
    Double_t current = 0;
    Double_t yval = 0;
    for(Int_t ib=0; ib<n; ++ib)
    {
        yval = ((TH1D*)fhChannel.At(foil_id))->GetBinContent(ib);
        if(yval > 0)
        {
            ++npos;
            current += yval;
        }
    }
    return current / Double_t(npos);
}

//----------------------------------------------------------------------------------
void MFoil::DetectSparks(Double_t xmax)
{
    // Definition of a spark: if 1 bin is 10X larger than the saturation current.
    // Needs fSatCurrent to be initialized.
    Int_t n = ((TH1D*)fhChannel.At(0))->FindBin(xmax);
    Int_t required_time = 1;
    
    for (Int_t ich = 0; ich < fNumChannels; ++ich)
    {
        Int_t count = 0;
        Int_t spark = 0;
        Double_t yspark = 0;
        Double_t xval = 0;
        Double_t yval = 0;

        for(int ib=1; ib<=n; ib++)
        {
            ((TH1D*)fhSparks.At(ich))->SetBinContent(ib, 0.0); // init
            yval = ((TH1D*)fhChannel.At(ich))->GetBinContent(ib);
            //if(TMath::Abs(yval) > 10.*TMath::Abs(fSatCurrent[ich]))
            if(TMath::Abs(yval) > 100)
            {
                ++count; 
                if(count > required_time && count < 10)
                {
                    ++spark;
                    // set the bins backwards
                    for(Int_t i=0; i<required_time; ++i)
                    {
                        xval = ((TH1D*)fhChannel.At(ich))->GetBinCenter(ib-i);
                        yspark = ((TH1D*)fhChannel.At(ich))->GetBinContent(ib-i);
                        ((TH1D*)fhSparks.At(ich))->SetBinContent(ib-i, yspark);
                    }
                }
            }
            else 
            { 
                count = 0; 
            }
        }
        fnSparks.push_back( spark );
    }   
}
//---------------------------------------------------------------------------------- 
Double_t MFoil::GetLastSparkPosition(Int_t foil_id)
{
    // Needs fhSparks to be initialized 
    Int_t n = ((TH1D*)fhSparks.At(foil_id))->FindBin(fMeasurementEnd-3);
    Double_t x = 0;
    Double_t y = 0;
    Double_t e = 0.01;
    for(int ib=1; ib<n; ib++)
    {
        y = ((TH1D*)fhSparks.At(foil_id))->GetBinContent(ib);
        // fhSparks is filled with zeros by default, finding the sparks is easy:
        if(TMath::Abs(y) > TMath::Abs(0+e)) x = ((TH1D*)fhSparks.At(foil_id))->GetBinCenter(ib);
    }
    return x;
}
//---------------------------------------------------------------------------------- 
Double_t MFoil::DetectMeasurementStop()
{
    // count consequitive bins with positive current
    // (negative here because of the flipped histogram)
    Int_t required_time=30; 
    Int_t lastbin = 1;
    Int_t n = ((TH1D*)fhChannelPos.At(0))->GetNbinsX();
    Double_t xmax[24];

    for (Int_t ich = 0; ich < fNumChannels; ++ich)
    {
        Int_t count=0;
        TH1D * h = ((TH1D*)fhChannelPos.At(ich));
        count=0;
        // start search from half
        for(int ib=n/2; ib<=n; ib++)
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
	return Form("I_{sat} = %.3f nA\n",fSatCurrent[foil_id]);
}
//----------------------------------------------------------------------------------
TString MFoil::GetInfoNumSparks(Int_t foil_id) const
{
    return Form("# of sparks = %d", fnSparks[foil_id]); 
}
//----------------------------------------------------------------------------------
TString MFoil::GetInFileName() const { return fInFileName; }
//----------------------------------------------------------------------------------
TString MFoil::GetName() const { return fName; }
//----------------------------------------------------------------------------------
void MFoil::CreateHLimit()
{
    fHLimit = new TH1D("hLimit", "", 1000, -1, 1E5);
    for(Int_t ib=1; ib<1000; ib++)
    {
	    fHLimit->SetBinContent(ib, 0.0); 
        fHLimit->SetBinError(ib, 0.5); // limit is 0.5 nA
    }
}
//----------------------------------------------------------------------------------
// Gets the color code of the foil 

// from Rtypes.h { kWhite =0,   kBlack =1,   kGray=920,
//                 kRed   =632, kGreen =416, kBlue=600, kYellow=400, kMagenta=616, kCyan=432,
//                 kOrange=800, kSpring=820, kTeal=840, kAzure =860, kViolet =880, kPink=900 };
Int_t MFoil::GetProcessedColor(Int_t ich) const 
{
    // 0=bad (red), 1=good (green), 2=problematic (orange)
    Int_t color;
    
    if(!fFlagIsProcessed) color=920;
    else 
    {
        switch(fFlagQuality.at(ich))
        {
            case 0: color=632+2; break; 
            case 1: color=416+2; break; 
            case 2: color=800+2; break; 
            default: color=920; break; 
        }
    }
    return color;
}
    
//----------------------------------------------------------------------------------
void MFoil::DrawHLimit(Int_t ich)
{   
    fHLimit->SetFillStyle(3003);
    fHLimit->SetFillColor(GetProcessedColor(ich));    
    fHLimit->SetLineColor(GetProcessedColor(ich));
    fHLimit->Draw("E3 same");
}
//----------------------------------------------------------------------------------
void  MFoil::DrawSatCurrent(Int_t ich)
{
    TLine * lcurrent = new TLine(fMeasurementStart, fSatCurrent.at(ich),fMeasurementEnd,  fSatCurrent.at(ich));
    lcurrent->SetLineColor(kBlack);
    lcurrent->SetLineWidth(3);
    lcurrent->Draw();
}
//----------------------------------------------------------------------------------
void  MFoil::DrawMeasurementRange(Int_t ich)
{
    TLine * lrangeLow = new TLine(fMeasurementStart, fSatCurrent.at(ich)/10., fMeasurementStart, fSatCurrent.at(ich)*10.);
    TLine * lrangeHigh= new TLine(fMeasurementEnd, fSatCurrent.at(ich)/10., fMeasurementEnd, fSatCurrent.at(ich)*10.);
    lrangeLow->SetLineWidth(2);
    lrangeHigh->SetLineWidth(2);
    
    lrangeLow->Draw();
    lrangeHigh->Draw();
}
//----------------------------------------------------------------------------------
void MFoil::DrawHChannel(Int_t ich, TString opt, TCanvas * c)
{
    TH1D * h = ((TH1D*)fhChannel.At(ich));

    SetPadMargins(c);
    SetAxisStyle(h);
    h->SetXTitle("time [sec]"); 
	h->SetYTitle("leakage current [nA]");
    h->SetLineColor(kBlue);

    h->Draw(opt);
    h->GetYaxis()->SetRangeUser(-1,1);
	c->Update();
}
//----------------------------------------------------------------------------------
void MFoil::SetPadMargins(TCanvas * c)
{
    TVirtualPad * p = c->GetPad(0);
    p->SetLeftMargin(0.15);
    p->SetRightMargin(0.06);
    p->SetTopMargin(0.06);
    p->SetBottomMargin(0.15);
	p->Update();
}
//----------------------------------------------------------------------------------
void MFoil::SetAxisStyle(TH1D * h)
{
    TAxis * ax = h->GetXaxis();
    TAxis * ay = h->GetYaxis();
    
    ax->SetTitleOffset(1.1);
    ax->SetLabelOffset(0.01);
    ay->SetTitleOffset(1.1);
    ay->SetLabelOffset(0.01);
    ax->SetTitleSize(0.06);
    ay->SetTitleSize(0.06);
}
//----------------------------------------------------------------------------------
void MFoil::DrawStdDev(Int_t ich, TString opt, TCanvas * c)
{
    TH1D * h = ((TH1D*)fhChStdDev.At(ich));
    h->SetXTitle("#sigma(I): median-value"); 
    h->SetYTitle("occurence"); 
    
    SetPadMargins(c);
    SetAxisStyle(h);
    
    //Double_t xmin = GetXlimits( ((TH1D*)fhChStdDev.At(ich)) )[0];
    //Double_t xmax = GetXlimits( ((TH1D*)fhChStdDev.At(ich)) )[1];
    //((TH1D*)fhChStdDev.At(ich))->GetXaxis()->SetLimits(xmin,xmax);
    //std::cout << "setting hist limits: " << xmin << " " << xmax << std::endl;
    h->Draw(opt);
    //((TH1D*)fhChStdDev.At(ich))->Print();
}
//----------------------------------------------------------------------------------
void MFoil::DrawSparks(Int_t ich, TString opt)
{
    ((TH1D*)fhSparks.At(ich))->SetLineColor(kRed);
    fhSparks.At(ich)->Draw(opt);
}


#endif
