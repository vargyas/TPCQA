#include "mleak.h"

#ifndef MLEAK_CXX
#define MLEAK_CXX


////////////////////////////////////////////////////////////////////////
/// \brief Default constructor
///
MLeak::MLeak(Int_t location) :
    fInFileName(0),
    fLocation(location),
    fName(0),
    fType(0),
    fNumChannels(24),
    fLimit(0),
    fInTree(0),
    fFlagIsProcessed(kFALSE),
    fFlagIsLoaded(kFALSE),
    fFlagQuality(0),
    fQualityText(0),
    fMeasurementStart(0),
    fMeasurementEnd(0),
    fhCurrentTime(0),
    fhCurrentTime1(0),
    fhCurrentTime2(0),
    fhCurrentTime3(0),
    fhCurrentStd(0),
    fSatCurrent(0),
    fhLimitTime(0),
    fhLimitStd(0)
{
    // default constructor
    fhCurrentTime.SetOwner(kTRUE);
    fhCurrentTime1.SetOwner(kTRUE);
    fhCurrentTime2.SetOwner(kTRUE);
    fhCurrentTime3.SetOwner(kTRUE);
    fhCurrentStd.SetOwner(kTRUE);

    if(fLocation == 0) fSaveDir = "~";
    if(fLocation == 1) fSaveDir = "~/cernbox/Work/ALICE/serviceWork/HV";
    else fSaveDir = "~";

    //CreateHLimitStd();
}

////////////////////////////////////////////////////////////////////////
/// \brief Default destructor
///
MLeak::~MLeak()
{
    if(fFlagIsLoaded)
    {
        // delete TList's objects
        fhCurrentTime.Clear();
        fhCurrentTime1.Clear();
        fhCurrentTime2.Clear();
        fhCurrentTime3.Clear();
        fhCurrentStd.Clear();

        delete fInTree;
    }
}

////////////////////////////////////////////////////////////////////////
/// \brief Generate name of output pdf
/// \return name of output pdf
///
TString MLeak::GetSaveName()
{
    return Form("%s/Report_HV_%s.pdf",fSaveDir.Data(),fName.Data());
}
TString MLeak::GetSaveDir() const
{
    return fSaveDir;
}

////////////////////////////////////////////////////////////////////////
/// \brief Guesses foil name by removing extension and "/"
///
void MLeak::GuessFoilName()
{
    fName = fInFileName;
    std::cout << "MLeak::GuessFoilName()..\t" << fInFileName << "\t" << fDataDir << std::endl;
    // remove absolute part of path
    fName.ReplaceAll(fDataDir,"");

    fName.ReplaceAll(".txt","");
    fName.ReplaceAll("/","");

    std::cout << "Guessed name of foil is: " << fName << std::endl;
}

////////////////////////////////////////////////////////////////////////
/// \brief Guesses the foil type (IROC/OROC1,2,3) and subtype (G1,2,3,4)
///
void MLeak::ProcessFileName()
{
    // determine type (and number of channels):
    if (fInFileName.Contains("I_") || fInFileName.Contains("I-")) {
        fType = 0;
        fNumChannels = 18;
    }
    else if (fInFileName.Contains("O1_") || fInFileName.Contains("O1-")) {
        fType = 1;
        fNumChannels = 20;
    }
    else if (fInFileName.Contains("O2_") || fInFileName.Contains("O2-")) {
        fType = 2;
        fNumChannels = 22;
    }
    else if (fInFileName.Contains("O3_") || fInFileName.Contains("O3-")) {
        fType = 3;
        fNumChannels = 24;
    }
    else {
        std::cerr << "Foil type not recognized!\n";
        fType = 4;
        fNumChannels = 24;
    }

    // determine subtype: protection resistors differ for different foils,
    // setting the accepted limit accordingly
    if (fInFileName.Contains("_G1_") || fInFileName.Contains("_G2_") || fInFileName.Contains("_G3_") ||
        fInFileName.Contains("-G1-") || fInFileName.Contains("-G2-") || fInFileName.Contains("-G3-") )
        fLimit = 0.160; // nA
    else if (fInFileName.Contains("_G4_") || fInFileName.Contains("-G4-"))
        fLimit = 0.046; // nA
    else fLimit = 0.5; // default in nA
}

////////////////////////////////////////////////////////////////////////
/// \brief Loads foil current file and fills the appropriate containers
/// \param filename
///
void MLeak::LoadFoilCurrents(const TString dirname, const TString filename)
{   
    std::cout << "MLeak::LoadFoilCurrents()\n";

    // setting the file name and determine the foil type from that
    fInFileName = filename;
    ProcessFileName();
    //fDataDir = gSystem->pwd();
    fDataDir = dirname;
    GuessFoilName();

    // not yet processed, setting flag
    fFlagIsProcessed = kFALSE;
    
    // clear histogram array before loading, if loaded already
    if (fFlagIsLoaded) {
        std::cout << "clearing histograms...\n";
        fhCurrentStd.Clear();
        fhCurrentTime.Clear();
        fhCurrentTime1.Clear();
        fhCurrentTime2.Clear();
        fhCurrentTime3.Clear();
    }

    std::cout << "read input tree\n";

    fInTree = new TTree("leakage_tree","Leakage current measurement");

    if(fLocation == 1)
    {
        std::cout << "Location: Budapest\n";
        fInTree->ReadFile(fInFileName,"time/D:I_00:I_01:I_02:I_03:I_04:I_05:I_06:I_07:I_08:I_09:I_10:I_11:I_12:I_13:I_14:I_15:I_16:I_17:I_18:I_19:I_20:I_21:I_22:I_23");
    }
    else if(fLocation == 0)
    {
        std::cout << "Location: Helsinki\n";
        // tree defined in the header of the input file:
        // Date/C:Time:TimeStamp/D:time:VMeas:IMeas:I_00:I_01:I_02:I_03:I_04:I_05:I_06:I_07:
        // I_08:I_09:I_10:I_11:I_12:I_13:I_14:I_15:I_16:I_17:I_18:I_19:I_20:I_21:I_22:I_23
        fInTree->ReadFile(fInFileName);
    }

    const Long64_t ndata = fInTree->GetEntries();
    Double_t timesD = 0;
    Double_t * currentsD = new Double_t(24);
    fInTree->SetBranchAddress("time",&timesD);

    // divide 3 regions (first 10 minutes, middle, last 10 minutes)
    const Long64_t itimes1=600, itimes2=ndata-2*600, itimes3=600;

    // easier to create graphs first, then convert them to histograms
    TGraph * g[4][24];

    for (Int_t ich = 0; ich<24; ich++)
    {
        fInTree->SetBranchAddress(Form("I_%.2d",ich), &currentsD[ich]);
        fhCurrentStd.Add( new TH1D(Form("hCurrentStd_CH%d", ich), Form("CH %d", ich), 1000, -fLimit, fLimit) );
        ((TH1D*)fhCurrentStd.At(ich))->SetXTitle("");
        ((TH1D*)fhCurrentStd.At(ich))->SetYTitle("");
        SetAxisStyle(((TH1D*)fhCurrentStd.At(ich)));

        g[0][ich] = new TGraph(ndata);
        g[1][ich] = new TGraph(itimes1);
        g[2][ich] = new TGraph(itimes2);
        g[3][ich] = new TGraph(itimes3);

        for(Int_t i=0; i<4; ++i) {
            g[i][ich]->SetTitle("");
            g[i][ich]->SetName(Form("gCurrentTime%d_CH%d",i,ich));
        }
    }

    Long64_t it1=0, it2=0, it3=0;
    fInTree->GetEvent(0);
    Double_t time0 = timesD; // save 0 time
    Double_t times_shift = 0;
    std::cout << "ndata = " << ndata << std::endl;

    for (Int_t ich = 0; ich<24; ich++) // load all, channels might be corrupted and skipped
    {
        it1=0; it2=0; it3=0;

        for( Long64_t idata=1; idata<ndata; idata++) // ignore first line (not evenly spaced)
        {
            fInTree->GetEvent(idata);
            times_shift = timesD-time0;

            ((TH1D*)fhCurrentStd.At(ich))->Fill(currentsD[ich]);
            g[0][ich]->SetPoint(idata,times_shift, currentsD[ich]);

            // Only fill detailed graphs if enough data
            if(ndata>1400)
            {
                if(times_shift<600.)
                    g[1][ich]->SetPoint(it1++, times_shift, currentsD[ich]);
                if(times_shift>600. && times_shift<(ndata-600.))
                    g[2][ich]->SetPoint(it2++, times_shift, currentsD[ich]);
                if(times_shift>(ndata-600.))
                    g[3][ich]->SetPoint(it3++, times_shift, currentsD[ich]);
            }
            // Fill with 0s otherwise
            else
            {
                std::cerr << "not enough data, filling detailed time histograms with zeros\n";
                g[1][ich]->SetPoint(it1++, times_shift, 0);
                g[2][ich]->SetPoint(it2++, times_shift, 0);
                g[3][ich]->SetPoint(it3++, times_shift, 0);
            }
        }

        fhCurrentTime.Add(  (TH1D*)ConstructHist(g[0][ich]) );
        fhCurrentTime1.Add( (TH1D*)ConstructHist(g[1][ich]) );
        fhCurrentTime2.Add( (TH1D*)ConstructHist(g[2][ich]) );
        fhCurrentTime3.Add( (TH1D*)ConstructHist(g[3][ich]) );

        ((TH1D*)fhCurrentTime.At(ich))->GetXaxis()->SetTimeDisplay(1);
        ((TH1D*)fhCurrentTime1.At(ich))->GetXaxis()->SetTimeDisplay(1);
        ((TH1D*)fhCurrentTime2.At(ich))->GetXaxis()->SetTimeDisplay(1);
        ((TH1D*)fhCurrentTime3.At(ich))->GetXaxis()->SetTimeDisplay(1);

        ((TH1D*)fhCurrentTime.At(ich))->GetXaxis()->SetTimeFormat("%H:%M");
        ((TH1D*)fhCurrentTime1.At(ich))->GetXaxis()->SetTimeFormat("%H:%M");
        ((TH1D*)fhCurrentTime2.At(ich))->GetXaxis()->SetTimeFormat("%H:%M");
        ((TH1D*)fhCurrentTime3.At(ich))->GetXaxis()->SetTimeFormat("%H:%M");

        // cleaning up
        // for(Int_t i=0; i<3; ++i) delete g[i][ich];
    }

    fFlagIsLoaded = kTRUE;
    //gROOT->ProcessLine(Form(".! say \"loadin foil %s done.\" ",fName.Data()));
}
//----------------------------------------------------------------------------------
void MLeak::ProcessFoilCurrents()
{   
    if(fFlagIsProcessed)
    {
        fSatCurrent.clear();
        fFlagQuality.clear();
    }
    
    // estimating saturation current
    for (Int_t ich = 0; ich < 24; ++ich)
    {
        fSatCurrent.push_back( ((TH1D*)fhCurrentStd.At(ich))->GetMean() );
    }
    // TODO: fit saturation current

    // determine foil quality
    for (Int_t ich = 0; ich < 24; ++ich)
    {
        if(fabs(fSatCurrent.at(ich))>=fLimit) fFlagQuality.push_back(0);        // bad foil
        else if(fabs(fSatCurrent.at(ich))<fLimit ) fFlagQuality.push_back(1);   // good foil
        else  fFlagQuality.push_back(2);                                        // strange foil
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

Int_t MLeak::GetLocation() const {return fLocation;}
//---------------------------------------------------------------------------------- 
Int_t MLeak::GetNC() { return fNumChannels; }
//----------------------------------------------------------------------------------
Int_t MLeak::GetType() { return fType; }
//----------------------------------------------------------------------------------
Bool_t MLeak::GetProcessedStatus() const { return fFlagIsProcessed; }
//----------------------------------------------------------------------------------
Bool_t MLeak::GetLoadedStatus() const { return fFlagIsLoaded; }
//----------------------------------------------------------------------------------
TString MLeak::GetInfoSatCurrent(Int_t foil_id) const
{
    return Form("I_{sat} = %.3f nA\n",fSatCurrent[foil_id]);
}//----------------------------------------------------------------------------------
TString MLeak::GetInfoLimitCurrent() const
{
    return Form("I_{limit} = %.3f nA\n",fLimit);
}
//----------------------------------------------------------------------------------
Double_t MLeak::GetSaturationCurrent(Int_t ich) { return fSatCurrent.at(ich); }

TString MLeak::GetInFileName() const { return fInFileName; }
//----------------------------------------------------------------------------------
TString MLeak::GetName() const { return fName; }
//----------------------------------------------------------------------------------
void MLeak::CreateHLimitTime()
{
    fhLimitTime = new TH1D("hLimitTime", "", 2000, fMeasurementStart, fMeasurementEnd);
    for(Int_t ib=1; ib<2000; ib++)
    {
        fhLimitTime->SetBinContent(ib, 0.0);
        fhLimitTime->SetBinError(ib, fLimit);
    }
    fhLimitTime->Print();
}
//----------------------------------------------------------------------------------
void MLeak::CreateHLimitStd(Int_t ich, Double_t ymax)
{
    fhLimitStd = new TH1D(Form("hLimitStd_CH%d",ich), "", 2000, -fLimit, fLimit);
    for(Int_t ib=1; ib<2000; ib++)
    {
        fhLimitStd->SetBinContent(ib, 0.0);
        fhLimitStd->SetBinError(ib, ymax); // limit is 0.5 nA
    }
}
//----------------------------------------------------------------------------------
// Gets the color code of the foil 

// from Rtypes.h { kWhite =0,   kBlack =1,   kGray=920,
//                 kRed   =632, kGreen =416, kBlue=600, kYellow=400, kMagenta=616, kCyan=432,
//                 kOrange=800, kSpring=820, kTeal=840, kAzure =860, kViolet =880, kPink=900 };
Int_t MLeak::GetProcessedColor(Int_t ich) const
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
void MLeak::DrawHLimitTime(Int_t ich)
{   
    fhLimitTime->SetFillStyle(3003);
    fhLimitTime->SetFillColor(GetProcessedColor(ich));
    fhLimitTime->SetLineColor(GetProcessedColor(ich));
    fhLimitTime->Draw("E3 same");
}
//----------------------------------------------------------------------------------
void MLeak::DrawHLimitStd(Int_t ich)
{
    fhLimitStd->SetFillStyle(3003);
    fhLimitStd->SetFillColor(GetProcessedColor(ich));
    fhLimitStd->SetLineColor(GetProcessedColor(ich));
    fhLimitStd->Draw("E3 same");
}

//----------------------------------------------------------------------------------
void  MLeak::DrawSatCurrentTime(Int_t ich)
{
    TLine * lcurrent = new TLine(fMeasurementStart, fSatCurrent.at(ich),fMeasurementEnd,  fSatCurrent.at(ich));
    lcurrent->SetLineColor(kBlack);
    lcurrent->SetLineWidth(3);
    lcurrent->Draw();
}
//----------------------------------------------------------------------------------
void  MLeak::DrawSatCurrentStd(Int_t ich)
{
    TLine * lcurrent = new TLine(fSatCurrent.at(ich),0, fSatCurrent.at(ich), ((TH1D*)fhCurrentStd.At(ich))->GetMaximum());
    lcurrent->SetLineColor(kBlack);
    lcurrent->SetLineWidth(1);
    lcurrent->SetLineStyle(2);
    lcurrent->Draw();
}

//----------------------------------------------------------------------------------
void  MLeak::DrawMeasurementRange(Int_t ich)
{
    TLine * lrangeLow = new TLine(fMeasurementStart, fSatCurrent.at(ich)/10., fMeasurementStart, fSatCurrent.at(ich)*10.);
    TLine * lrangeHigh= new TLine(fMeasurementEnd, fSatCurrent.at(ich)/10., fMeasurementEnd, fSatCurrent.at(ich)*10.);
    lrangeLow->SetLineWidth(2);
    lrangeHigh->SetLineWidth(2);
    
    lrangeLow->Draw();
    lrangeHigh->Draw();
}
//----------------------------------------------------------------------------------
void MLeak::DrawLimitStd(Int_t ich)
{
    TLine * low  = new TLine(-fLimit, 0, -fLimit, ((TH1D*)fhCurrentStd.At(ich))->GetMaximum() );
    TLine * high = new TLine(fLimit, 0, fLimit, ((TH1D*)fhCurrentStd.At(ich))->GetMaximum() );
    low->Draw();
    high->Draw();
}

//----------------------------------------------------------------------------------

void MLeak::DrawLimitTime()
{
    TLine * low  = new TLine(fMeasurementStart, -fLimit, fMeasurementEnd, -fLimit );
    TLine * high = new TLine(fMeasurementStart, fLimit, fMeasurementEnd, fLimit );
    low->SetLineWidth(1);
    low->SetLineStyle(2);
    high->SetLineWidth(1);
    high->SetLineStyle(2);
    low->Draw();
    high->Draw();
}
//----------------------------------------------------------------------------------


void MLeak::DrawCurrentTime(Int_t itab, Int_t ich, TPad * p, Bool_t drawaxes)
{
    gStyle->SetOptStat(0);
    TH1D * h;
    switch(itab)
    {
        case 1: h = ((TH1D*)fhCurrentTime.At(ich)); break;
        case 2: h = ((TH1D*)fhCurrentTime1.At(ich)); break;
        case 3: h = ((TH1D*)fhCurrentTime2.At(ich)); break;
        case 4: h = ((TH1D*)fhCurrentTime3.At(ich)); break;
        default: h = ((TH1D*)fhCurrentTime.At(ich)); break;
    }
    h->SetLineColor( GetProcessedColor(ich) );

    if(drawaxes)
    {
        SetAxisStyle(h);
        h->GetXaxis()->CenterTitle();
        h->GetYaxis()->CenterTitle();
        h->GetXaxis()->SetTitle("time [hour:min]");
        h->GetYaxis()->SetTitle("Leakage current [nA]");
    }

    h->Draw("L");
    if(itab==1 || itab==3) h->GetYaxis()->SetRangeUser(-2.*fLimit,2.*fLimit); // overview and measurement
    //if(itab==2 || itab==4) g->GetYaxis()->SetRangeUser(-2.*fLimit,2.*fLimit); // measurement start / end

    if(GetProcessedStatus())
    {
        DrawLimitTime();

        DrawSatCurrentTime(ich);

        if(drawaxes)
        {
            TLegend * tinfo = new TLegend(0.2, 0.2, 0.8, 0.3, "", "brNDC");
            tinfo->SetFillStyle(0); tinfo->SetBorderSize(0); tinfo->SetTextSize(0.05);
            tinfo->AddEntry((TObject*)0,GetInfoSatCurrent(ich), "");
            tinfo->AddEntry((TObject*)0,GetInfoLimitCurrent(), "");
            tinfo->Draw();

            TLegend * tch = new TLegend(0.2, 0.75, 0.8, 0.98, "", "brNDC");
            tch->SetFillStyle(0); tch->SetBorderSize(0); tch->SetTextSize(0.1);
            tch->AddEntry((TObject*)0,Form("CH %d",ich), "");
            tch->Draw();
        }
        else
        {
            TLegend * tch = new TLegend(0.2, 0.75, 0.8, 0.98, "", "brNDC");
            tch->SetFillStyle(0); tch->SetBorderSize(0); tch->SetTextSize(0.15);
            tch->AddEntry((TObject*)0,Form("CH %d",ich), "");
            tch->Draw();
        }
    }
    p->Modified();
    p->Update();
}

//----------------------------------------------------------------------------------
void MLeak::DrawCurrentStd(Int_t ich, TPad * p, Bool_t drawaxes)
{
    std::cout << " MLeak::DrawCurrentStd(Int_t ich, TPad * p, Bool_t drawaxes) done...\n";
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);

    TH1D * h = ((TH1D*)fhCurrentStd.At(ich));
    h->SetLineColor( GetProcessedColor(ich) );
    h->SetMarkerColor( GetProcessedColor(ich) );

    if(drawaxes)
    {
        SetAxisStyle(h);
        h->GetXaxis()->CenterTitle();
        h->GetYaxis()->CenterTitle();
        h->GetXaxis()->SetTitle("leakage current [nA]");
        h->GetYaxis()->SetTitle("occurence");
    }

    h->Draw("HIST");

    if(GetProcessedStatus())
    {
        DrawLimitStd(ich);

        DrawSatCurrentStd(ich);

        if(drawaxes)
        {
            TLegend * tch = new TLegend(0.2, 0.7, 0.5, 0.8);
            //tch->SetFillStyle(3001); tch->SetFillColor(kWhite);
            tch->SetBorderSize(0); tch->SetTextSize(0.1);
            tch->AddEntry((TObject*)0,Form("CH %d",ich), "");
            tch->Draw();

            TLegend * tinfo = new TLegend(0.2, 0.3, 0.5, 0.4);
            //tinfo->SetFillStyle(3001); tinfo->SetFillColor(kWhite);
            tinfo->SetBorderSize(0); tinfo->SetTextSize(0.05);
            tinfo->AddEntry((TObject*)0,GetInfoSatCurrent(ich), "");
            tinfo->AddEntry((TObject*)0,GetInfoLimitCurrent(), "");
            tinfo->Draw();
        }
        else
        {
            TLegend * tch = new TLegend(0.1, 0.7, 0.2, 0.8);
            //tch->SetFillStyle(3001); tch->SetFillColor(kWhite);
            tch->SetBorderSize(0); tch->SetTextSize(0.15);
            tch->AddEntry((TObject*)0,Form("CH %d",ich), "");
            tch->Draw();
        }
    }

    p->Modified();
    p->Update();
}

//----------------------------------------------------------------------------------
void MLeak::SetPadMargins(TCanvas * c)
{
    TVirtualPad * p = c->GetPad(0);
    p->SetLeftMargin(0.15);
    p->SetRightMargin(0.06);
    p->SetTopMargin(0.06);
    p->SetBottomMargin(0.15);
    p->Update();
}
//----------------------------------------------------------------------------------
void MLeak::SetAxisStyle(TGraph * h)
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
void MLeak::SetAxisStyle(TH1D * h)
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

/*TCanvas * MLeak::DrawSatCurrentTable()
{
    TCanvas * csat = new TCanvas("csat", "", 800, 600);
    csat->cd();

    TPaveLabel * plsat[24];

    for(Int_t ich=0; ich<fNumChannels; ich++)
    {
        TString textsat = Form("CH %.2d \t %.4f nA",ich, fSatCurrent.at(ich) );
        plsat[ich] = new TPaveLabel(0.0, 100./Double_t(fNumChannels)*ich, 100, 100./Double_t(fNumChannels)*(ich+1), textsat);
        plsat[ich]->SetBorderSize(0); plsat[ich]->SetFillColor(kWhite);
        //ytitle[itab]->SetTextSize(20./(fPad[1][itab]->GetBBox().fHeight));
        plsat[ich]->SetTextFont(42);
        plsat[ich]->Draw();
    }
    return csat;
}*/


#endif // MLEAK_CXX
