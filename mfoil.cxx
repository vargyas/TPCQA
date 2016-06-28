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
        for (Int_t ich = 0; ich<fNumChannels; ich++)
        {
            delete fhChannel.At(ich);
            delete fhChannelPos.At(ich);
            delete fhChStdDev.At(ich);
            delete fhSparks.At(ich);
        }
        fhChannel.Clear(); 
        fhChannelPos.Clear(); 
        fhChStdDev.Clear();
        fhSparks.Clear();
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
        for (Int_t ich = 0; ich < 24; ich++) { inFile >> c[ich]; } // file contains 24 values regardless of actual channels

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
        fhSparks.Add(new TH1D(Form("h_spark%d", ich+1), Form("CH %d", ich+1), ndata+1, -0.25, double(ndata)/2. + 0.25));
        
        for(Int_t it=0; it<ndata; it++)
        {
            ((TH1D*)fhChannel.At(ich))->SetBinContent(it+1, fCurrents[it][ich]);
            ((TH1D*)fhChannel.At(ich))->SetBinError(it+1, 1E-10);
            //((TH1D*)fhChannelPos.At(ich))->SetBinContent(it+1, TMath::Abs(fCurrents[it][ich]));
            ((TH1D*)fhChannelPos.At(ich))->SetBinContent(it+1, -1.*(fCurrents[it][ich]));
            ((TH1D*)fhChannelPos.At(ich))->SetBinError(it+1, 1E-10);
        }

        // create and fill std dev. histogram
        Double_t val = 0;
        Double_t median = GetMedian( (TH1D*)fhChannelPos.At(ich) );
        fhChStdDev.Add(new TH1D(Form("h_stddev%d", ich+1), Form("CH %d", ich+1), ndata+1, -3.*TMath::Abs(median), 3.*TMath::Abs(median)));
        for(Int_t it=0; it<ndata; it++)
        {
            val = ((TH1D*)fhChannelPos.At(ich))->GetBinContent(it+1);
            ((TH1D*)fhChStdDev.At(ich))->Fill( (median-val) );
        }
    }
    fFlagIsLoaded = kTRUE;
}
//----------------------------------------------------------------------------------
void MFoil::ProcessFoilCurrents()
{   
    if(fFlagIsProcessed)
        fSatCurrent.clear();

    // get measurement range:
    fMeasurementEnd   = DetectMeasurementStop();
    fMeasurementStart = fMeasurementEnd - 200;

    // add spark counting, and remove those before fitting
    
    // detect saturation current by averaging measurement area
    for (Int_t ich = 0; ich < fNumChannels; ++ich)
    {
        Int_t istart  = ((TH1D*)fhChannelPos.At(ich))->FindBin(fMeasurementStart);
        Int_t istop   = ((TH1D*)fhChannelPos.At(ich))->FindBin(fMeasurementEnd);
        Int_t count   = 0;
        Double_t yval = 0;
        Double_t current = 0;

        for(Int_t ib=istart; ib<istop; ++ib)
        {
            yval = ((TH1D*)fhChannelPos.At(ich))->GetBinContent(ib);
            if(yval>0)
            {
                ++count;
                current+=yval;
            }
        }
        fSatCurrent.push_back( current/Double_t(count) );
    }   
    DetectNSparks();

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
/*Double_t MFoil::DetectMeasurementStart()
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
*/
//----------------------------------------------------------------------------------
// write own peak detector instead of TSpectrum
Double_t MFoil::DetectMeasurementStart()
{ 
    return 22;
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
        yval = ((TH1D*)fhChannelPos.At(foil_id))->GetBinContent(ib);
        if(yval > 0)
        {
            ++npos;
            current += yval;
        }
    }
    return current / Double_t(npos);
}

//----------------------------------------------------------------------------------
void MFoil::DetectNSparks()
{
    // Definition of a spark: if 1 bin is 10X larger than the saturation current.
    // Needs fSatCurrent to be initialized.
    Int_t n = ((TH1D*)fhChannelPos.At(0))->GetNbinsX();
    Int_t required_time = 1;
    
    for (Int_t ich = 0; ich < fNumChannels; ++ich)
    {
        Int_t count = 0;
        Int_t spark = 0;
        Double_t yspark = 0;
        Double_t yval = 0;
        //Double_t mean = EstimateSatCurrent(ich);
        for(int ib=1; ib<=n; ib++)
        {
            ((TH1D*)fhSparks.At(ich))->SetBinContent(ib, 0.0); // init
            yval = ((TH1D*)fhChannelPos.At(ich))->GetBinContent(ib);
            if(yval > 10.*fSatCurrent[ich])
            {
                ++count; 
                if(count > required_time && count < 10)
                {
                    if(count == required_time+1) ++spark;
                    // set the bins backwards
                    for(Int_t i=0; i<required_time; ++i)
                    {
                        yspark = ((TH1D*)fhChannelPos.At(ich))->GetBinContent(ib-i);
                        ((TH1D*)fhSparks.At(ich))->SetBinContent(ib-i, yspark);
                    }
                }
            }
            else { 
                count = 0; 
            }
        }
        fnSparks.push_back( spark );
    }   
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
    return Form("I_{sat} = %.3f nA\n",fSatCurrent[foil_id]*1E9);
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
void MFoil::DrawHChannel(Int_t ich, TString opt)
{
    ((TH1D*)fhChannelPos.At(ich))->SetLineColor(kBlue);
    fhChannelPos.At(ich)->Draw(opt);
}
//----------------------------------------------------------------------------------
void MFoil::DrawStdDev(Int_t ich, TString opt)
{
    //Double_t xmin = GetXlimits( ((TH1D*)fhChStdDev.At(ich)) )[0];
    //Double_t xmax = GetXlimits( ((TH1D*)fhChStdDev.At(ich)) )[1];
    //((TH1D*)fhChStdDev.At(ich))->GetXaxis()->SetLimits(xmin,xmax);
    //std::cout << "setting hist limits: " << xmin << " " << xmax << std::endl;
    ((TH1D*)fhChStdDev.At(ich))->Draw(opt);
    //((TH1D*)fhChStdDev.At(ich))->Print();
}
//----------------------------------------------------------------------------------
void MFoil::DrawSparks(Int_t ich, TString opt)
{
    ((TH1D*)fhSparks.At(ich))->SetLineColor(kRed);
    fhSparks.At(ich)->Draw(opt);
}


#endif
