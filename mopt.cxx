
#ifndef MOPT_CXX
#define MOPT_CXX

#include "mopt.h"
#include <sys/stat.h>
//#include <fstream>


//----------------------------------------------------------------------------------
bool FileExists(char* filename)
{
    struct stat fileInfo;
    return stat(filename, &fileInfo) == 0;
}
//----------------------------------------------------------------------------------
MOpt::MOpt(Int_t location)
{
    fLocation = location;
    fConv = 4.34/1000.; // conversion from pixel to millimeter
    for(Int_t i=0; i<2; i++)
    {
        fIsLoaded[i]=kFALSE;
        fInFileName[i]="";
    }
}
//----------------------------------------------------------------------------------
MOpt::~MOpt() // TODO: delete also histograms/trees
{
    std::cout << "\t Deleting MOpt()...\n";
    // destructor
    //if(fInFile[0]->IsOpen()) fInFile[0]->Close();
    //if(fInFile[1]->IsOpen()) fInFile[1]->Close();
}
//----------------------------------------------------------------------------------
void MOpt::GuessFoilNameRoot(const TString name)
{
    std::cout << "Guessing foil name from ROOT file\n";

    TString tmpname(name);
    // remove extension
    fName = tmpname.ReplaceAll(".root","");
    // remove absolute path
    tmpname.Remove(fName.Last('/'));
    fName.ReplaceAll(tmpname,"");
    // remove remaining garbage
    fName.ReplaceAll("/","");

    std::cout << "MOpt::Guessed foil name is: " << fName << std::endl;
}

//----------------------------------------------------------------------------------
void MOpt::GuessFoilNameDir(const TString name)
{
    std::cout << "Guessing foil name from directory\n";
    TString tmpname = name;
    cout << tmpname << endl;
    // first remove results_conv folder
    tmpname.ReplaceAll("/results_conv4","");
    fName = tmpname;

    // remove absolute part of path
    tmpname.Remove(fName.Last('/'));
    fName.ReplaceAll(tmpname,"");

    // remove budapest conventions
    if(fLocation == 1)
    {
        fName.ReplaceAll("data_","");
        fName.ReplaceAll("-s-1","");
        fName.ReplaceAll("-u-1","");
        fName.ReplaceAll("/","");

    }
    // remove helsinki conventions
    if(fLocation == 0)
    {
        fName.ReplaceAll("_S","");
        fName.ReplaceAll("_U","");

        Ssiz_t dash = fName.Last('-');
        Ssiz_t underscore = fName.Last('_');
        if(dash>-1) {
            if(dash>underscore) { // ensure removing date string after last dash
                fName.Remove(dash);
            }
            else fName.Remove(underscore);
        }
        fName.ReplaceAll("/","");
        fName.ReplaceAll("_","-"); // keep output unified
    }
    std::cout << "MOpt::Guessed foil name is: " << fName << std::endl;
}

void MOpt::GuessFoilType()
{
    TString name="";
    if(fInFileName[0]!="")  name = fInFileName[0];
    else if(fInFileName[1]!="")  name = fInFileName[1];


    // determine type (and number of channels):
    if (name.Contains("I_") || name.Contains("I-")) {
        fType = 0;
    }
    else if (name.Contains("O1_") || name.Contains("O1-")) {
        fType = 1;
    }
    else if (name.Contains("O2_") || name.Contains("O2-")) {
        fType = 2;
    }
    else if (name.Contains("O3_") || name.Contains("O3-")) {
        fType = 3;
    }
    else {
        std::cerr << "Foil type not recognized!\n";
        fType = 0; // temporary fix for prediction IROC's with strange name
    }
    std::cout << "MOpt::Guessed foil type is: " << fType << std::endl;

    // determine subtype: protection resistors differ for different foils,
    // setting the accepted limit accordingly
    //if (fInFileName.Contains("_G1_") || fInFileName.Contains("_G2_") || fInFileName.Contains("_G3_") ||
    //    fInFileName.Contains("-G1-") || fInFileName.Contains("-G2-") || fInFileName.Contains("-G3-") )
    //    fLimit = 0.160; // nA
    //else if (fInFileName.Contains("_G4_"))
    //    fLimit = 0.046; // nA
    //else fLimit = 0.5; // default in nA
}
//----------------------------------------------------------------------------------
void MOpt::LoadFile(const TString filename, const TString outputdir, Int_t which_side)
{
    fInFileName[which_side] = filename;

	fOutDir[which_side] = outputdir;
    fDataDir[which_side] = gSystem->pwd();
    if(fInFileName[which_side].Contains(".root"))
        GuessFoilNameRoot(filename);
    else
        GuessFoilNameDir(fDataDir[which_side]);

    GuessFoilType();

    // If h5 extension is selected, run converter to create ROOT file with TTree
    if( filename.Contains(".h5"))
    {
        // This program creates a ROOT tree from the optical scan's hdf5 file output
		const TString bash_command = Form(".! python ~/Dropbox/TPCQA/utils/loadHDF5tree.py %s %s",fDataDir[which_side].Data(),GetROOTName(which_side).Data());
        std::cout << "Processing command:\n" << bash_command << std::endl;
        gROOT->ProcessLine(bash_command);

		//fInFileName[which_side] = fDataDir[which_side]+"/outfile.root";
		fInFileName[which_side] = GetROOTName(which_side).Data();
    }

    fInFile[which_side] = TFile::Open(fInFileName[which_side]);
    fInFile[which_side]->ls();

    fTree[which_side][kInner] = (TTree*) fInFile[which_side]->Get("inner_tree");
    fTree[which_side][kInner]->SetName(Form("inner_tree_%d", which_side));
    fTree[which_side][kOuter] = (TTree*) fInFile[which_side]->Get("outer_tree");
    fTree[which_side][kOuter]->SetName(Form("outer_tree_%d", which_side));

    fIsLoaded[which_side] = kTRUE;

    CreateOutputContainers(which_side);

    FillOutputContainers(which_side);
}
//----------------------------------------------------------------------------------
void MOpt::CloseFile(Int_t which_side)
{
    std::cout << "MOpt::CloseFile()\n";
    for(Int_t j=0; j<2; j++)
    {
        delete fhMapDiam[which_side][j];
        delete fhMapStd[which_side][j];
        delete fhMapN[which_side][j];
    }
    for(Int_t j=0; j<3; j++) {
        delete fhProfDiam[which_side][j];
    }
    delete fhMapRim[which_side];
    fInFile[which_side]->Close();
}
//----------------------------------------------------------------------------------
void MOpt::CreateOutputContainers(Int_t which_side)
{
    std::cout << "CreateOutputContainers() for side: " << which_side << std::endl;
    TString holtyp[] = {"inner", "outer"};
    Int_t i = which_side;

    // setup bin sizes
    Double_t xstep = 1; // bin width in x [mm]
    Double_t ystep = 1; // bin width in y [mm]

    // histogram sizes are the blueprint sizes in mm with the copper area included
    // iroc,  oroc1, oroc2, oroc3, unrecognised (largest is chosen)
    Double_t xx[] = {467.0, 595.8, 726.2, 867.0, 867.0};
    Double_t yy[] = {496.5, 353.0, 350.0, 379.0, 379.0};

    // correction to increase above sizes
    // (nbins will grow accordingly, keeping the distance ratio)
    Double_t corrx=0.05;
    Double_t corry=0.05;

    Int_t nx[5], ny[5];
    for(Int_t i=0; i<5; ++i) {
        //xx[i]=xx[i]*corrx;
        //yy[i]=yy[i]*corry;
        nx[i] = xx[i]*(1+corrx)/xstep;
        ny[i] = yy[i]*(1+corry)/ystep;
    }


    for(Int_t j=0; j<2; j++)
    {
        // might need adjustment -yy[fType]+offset, offset
        fhMapDiam[i][j]  = new TH2D(Form("hmap_diam_%d_%d",i,j),Form("%s diameter",holtyp[j].Data()),         nx[fType], -xx[fType]*corrx, xx[fType], ny[fType], -yy[fType], yy[fType]*corry);
        fhMapStd[i][j]   = new TH2D(Form("hmap_std_%d_%d",i,j), Form("std. of %s diameter",holtyp[j].Data()), nx[fType], -xx[fType]*corrx, xx[fType], ny[fType], -yy[fType], yy[fType]*corry);
        fhMapN[i][j]     = new TH2D(Form("hmap_n_%d_%d",i,j),   Form("N %s",holtyp[j].Data()),                nx[fType], -xx[fType]*corrx, xx[fType], ny[fType], -yy[fType], yy[fType]*corry);
    }
    fhProfDiam[i][kInner] = new TH1D(Form("hprof_diam_%d_%d",i,kInner),"",300,0,110);
    fhProfDiam[i][kOuter] = new TH1D(Form("hprof_diam_%d_%d",i,kOuter),"",300,0,110);
    fhProfDiam[i][kRim]   = new TH1D(Form("hprof_diam_%d_%d",i,kRim),"",300,0,110);
    for(Int_t j=0; j<3; j++)
    {
        fhProfDiam[i][j]->GetXaxis()->SetTitle("diameter [#mum]");
        fhProfDiam[i][j]->GetYaxis()->SetTitle("occurrence");
    }
    fhMapLight[i] = new TH2D(Form("hmap_fl_%d",i), Form("Foreground light"), nx[fType], -xx[fType]*corrx, xx[fType], ny[fType], -yy[fType], yy[fType]*corry);

    ffProfFit[i][kInner] = new TF1(Form("inner_fit_%d", which_side),"gaus",0,110);
    ffProfFit[i][kOuter] = new TF1(Form("outer_fit_%d", which_side),"gaus",0,110);
    ffProfFit[i][kRim]   = new TF1(Form("rim_fit_%d", which_side),"gaus",0,110);
}
//----------------------------------------------------------------------------------
void MOpt::FillOutputContainers(Int_t which_side)
{
    std::cout << "MOpt::FillOutputContainers()\n" << std::endl;

    fTree[which_side][kInner]->Draw(Form("x*4.34/1000:y*4.34/1000>>hmap_diam_%d_%d",which_side,kInner),"d*4.34","goff");
    fTree[which_side][kOuter]->Draw(Form("x*4.34/1000:y*4.34/1000>>hmap_diam_%d_%d",which_side,kOuter),"d*4.34","goff");

    fTree[which_side][kInner]->Draw(Form("x*4.34/1000:y*4.34/1000>>hmap_n_%d_%d",which_side,kInner),"","goff");
    fTree[which_side][kOuter]->Draw(Form("x*4.34/1000:y*4.34/1000>>hmap_n_%d_%d",which_side,kOuter),"","goff");

    fTree[which_side][kInner]->Draw(Form("x*4.34/1000:y*4.34/1000>>hmap_fl_%d",which_side),"fl","goff");
    fTree[which_side][kOuter]->Draw(Form("x*4.34/1000:y*4.34/1000>>hmap_fl_%d",which_side),"fl","goff");

    fhMapDiam[which_side][kInner]->Divide(fhMapN[which_side][kInner]);
    fhMapDiam[which_side][kInner]->GetZaxis()->SetRangeUser(40, 70);

    fhMapDiam[which_side][kOuter]->Divide(fhMapN[which_side][kOuter]);
    fhMapDiam[which_side][kOuter]->GetZaxis()->SetRangeUser(60, 100);

    fhMapLight[which_side]->Divide(fhMapN[which_side][kInner]);
    fhMapLight[which_side]->GetZaxis()->SetRangeUser(140, 180);

    fTree[which_side][kInner]->Draw(Form("d*4.34>>hprof_diam_%d_%d",which_side,kInner),"", "goff");
    fTree[which_side][kOuter]->Draw(Form("d*4.34>>hprof_diam_%d_%d",which_side,kOuter),"", "goff");
    //fTree[which_side][kRim]->Draw(Form("rim*4.34>>hprof_diam_%d_%d",which_side,kRim),"", "goff");

    fhProfDiam[which_side][kInner]->Fit(ffProfFit[which_side][kInner], "RNQ","",40,80);
    fhProfDiam[which_side][kOuter]->Fit(ffProfFit[which_side][kOuter], "RNQ","",60,100);

    CalculateStd(which_side, kInner);
    CalculateStd(which_side, kOuter);

    CalculateRim(which_side);

}
//----------------------------------------------------------------------------------
void MOpt::CalculateRim(Int_t which_side)
{
    Int_t ni, no, n;
    Double_t inner, outer, rim;

    fhMapRim[which_side] = (TH2D*) fhMapDiam[which_side][0]->Clone(Form("hmap_rim_%d",which_side));
    fhMapRim[which_side]->Reset();

    for(Int_t ibx=1; ibx<=fhMapRim[which_side]->GetNbinsX(); ++ibx)
    {
        for(Int_t iby=1; iby<=fhMapRim[which_side]->GetNbinsY(); ++iby)
        {
            ni = fhMapN[which_side][kInner]->GetBinContent(ibx, iby);
            no = fhMapN[which_side][kOuter]->GetBinContent(ibx, iby);
            if(ni<no) n=ni;
            else n=no;

            inner = fhMapDiam[which_side][kInner]->GetBinContent(ibx, iby);
            outer = fhMapDiam[which_side][kOuter]->GetBinContent(ibx, iby);
            rim = (outer-inner)/2.;
            fhMapRim[which_side]->SetBinContent(ibx, iby, rim);
            fhProfDiam[which_side][kRim]->Fill(rim, n);
        }
    }
    Double_t mean = fhProfDiam[which_side][kRim]->GetMean();
    fhMapRim[which_side]->GetZaxis()->SetRangeUser(mean-3, mean+3);
    fhMapRim[which_side]->SetTitle("rim width");
    //fhMapRim[which_side]->Divide(fhMapN[which_side][kOuter]);
    fhProfDiam[which_side][kRim]->Scale(1./3.);
    fhProfDiam[which_side][kRim]->Fit(ffProfFit[which_side][kRim], "RNQ","",0,20);
}
//----------------------------------------------------------------------------------
void MOpt::CalculateStd(Int_t which_side, Int_t which_hole)
{
    // needs mean histogram
    TTreeReader reader(fTree[which_side][which_hole]->GetName(), fInFile[which_side]);
    TTreeReaderValue<Double_t> d_t(reader, "d");
    TTreeReaderValue<Double_t> x_t(reader, "y"); // this is on purpose, I don't know why it does flip x-y though
    TTreeReaderValue<Double_t> y_t(reader, "x"); // this is on purpose, I don't know why it does flip x-y though

    Int_t globalBin = 0;
    Double_t mean = 0;
    Int_t counter = 0;
    while(reader.Next()) {
        globalBin = fhMapStd[which_side][which_hole]->FindBin(*x_t*4.34/1000., *y_t*4.34/1000.);
        mean = fhMapDiam[which_side][which_hole]->GetBinContent(globalBin);

        fhMapStd[which_side][which_hole]->Fill(*x_t*4.34/1000., *y_t*4.34/1000., pow((*d_t*4.34-mean),2.));
        //if(counter<20000) {
        //  cout << globalBin << "\t" << *x_t*4.34/1000. << "\t" << *y_t*4.34/1000. << "\t" << mean  << "\t" << *d_t*4.34 << endl;
        //  ++counter;
        //}
    }
    fhMapStd[which_side][which_hole]->Divide(fhMapN[which_side][which_hole]);
    fhMapStd[which_side][which_hole]->Print();
    fhMapStd[which_side][which_hole]->GetZaxis()->SetRangeUser(0,4);
}

void MOpt::GetOrigoShift(Int_t which_side)
{
    // Finding the middle of the foil, that would be the vector
    // Get x first (easy because it is symmetric):
    TH1D * hx = (TH1D*) fhMapN[which_side][kInner]->ProjectionX("hx")->Clone();
    fShift[0] = hx->GetMean();

    // Get y. More complicated because of asymmetry in sector boundaries (GetMean would
    // be slightly off).
    Int_t meanbin = fhMapN[which_side][kInner]->GetXaxis()->FindBin(fShift[0]);
    TH1D * hy = (TH1D*) fhMapN[which_side][kInner]->ProjectionY("hy",meanbin-10,meanbin+10)->Clone();
    Int_t startBin = hy->FindFirstBinAbove(200);
    Int_t stopBin = hy->FindLastBinAbove(200);
    Double_t start = hy->GetBinCenter(startBin);
    Double_t stop = hy->GetBinCenter(stopBin);
    fShift[1] = (stop+start)/2.;
}

//----------------------------------------------------------------------------------
TString MOpt::GetFitResult(Int_t which_side, Int_t which_hole)
{
    TF1 * f = (TF1*) ffProfFit[which_side][which_hole];
    return Form("d = %.0f #pm %.1f #mum", f->GetParameter(1), f->GetParameter(2) );
}
//----------------------------------------------------------------------------------
TString MOpt::GetMeanWidth(Int_t which_side, Int_t which_hole)
{
    // note that GetRMS() returns the standard deviation
    return Form("d = %.0f #pm %.1f #mum", fhProfDiam[which_side][which_hole]->GetMean(), fhProfDiam[which_side][which_hole]->GetRMS() );
}

//----------------------------------------------------------------------------------
void MOpt::DrawFitResult(Int_t which_side)
{
    gROOT->SetStyle("Modern");

    TLegend * tinfoh = new TLegend(0.75, 0.6, 0.9, 0.89, "", "brNDC");
    tinfoh->SetFillStyle(0); tinfoh->SetBorderSize(0); tinfoh->SetTextSize(0.05);
    tinfoh->AddEntry((TObject*)0,GetMeanWidth(which_side, kRim), "");
    tinfoh->AddEntry((TObject*)0,GetMeanWidth(which_side, kInner), "");
    tinfoh->AddEntry((TObject*)0,GetMeanWidth(which_side, kOuter), "");
    tinfoh->Draw();

    TLegend * tleg = new TLegend(0.6, 0.6, 0.75, 0.89, "", "brNDC");
    tleg->SetFillStyle(0); tleg->SetBorderSize(0); tleg->SetTextSize(0.05);
    tleg->AddEntry(fhProfDiam[which_side][kRim], "#color[634]{rim (/3.)}", "");
    tleg->AddEntry(fhProfDiam[which_side][kInner],"#color[417]{inner}", "");
    tleg->AddEntry(fhProfDiam[which_side][kOuter],"#color[601]{outer}", "");
    tleg->Draw();

    TLegend * tinfo = new TLegend(0.75, 0.6, 0.9, 0.89, "", "brNDC");
    tinfo->SetFillStyle(0); tinfo->SetBorderSize(0); tinfo->SetTextSize(0.05);
    tinfo->AddEntry((TObject*)0,GetMeanWidth(which_side, kRim), "");
    tinfo->AddEntry((TObject*)0,GetMeanWidth(which_side, kInner), "");
    tinfo->AddEntry((TObject*)0,GetMeanWidth(which_side, kOuter), "");
    tinfo->Draw();

}

//----------------------------------------------------------------------------------
void MOpt::DrawMaps(TPad * p, Int_t which_side, Int_t which_hole, Int_t which_histo)
{
    p->cd();
    if(which_histo==0) fhMapDiam[which_side][which_hole]->Draw("colz");
    if(which_histo==1) fhMapStd[which_side][which_hole]->Draw("colz");
    if(which_histo==2) fhMapN[which_side][which_hole]->Draw("colz");
    if(which_histo==3) fhMapRim[which_side]->Draw("colz");
    if(which_histo==4) fhMapLight[which_side]->Draw("colz");

    DrawOrigoShift(which_side);
    DrawFrame();

    p->Modified();
    p->Update();
}
void MOpt::DrawOrigoShift(Int_t which_side)
{
    GetOrigoShift(which_side);

    TLine * ls[4];
    ls[0] = new TLine(fShift[0], fhMapN[which_side][0]->GetYaxis()->GetBinCenter(0), fShift[0], fhMapN[which_side][0]->GetYaxis()->GetBinCenter(fhMapN[which_side][0]->GetNbinsY()));
    ls[1] = new TLine(fhMapN[which_side][0]->GetXaxis()->GetBinCenter(0), fShift[1], fhMapN[which_side][0]->GetXaxis()->GetBinCenter(fhMapN[which_side][0]->GetNbinsX()), fShift[1]);
    ls[0]->Draw();
    ls[1]->Draw();
}
void MOpt::DrawFrame()
{

    // blueprint sizes in mm with the copper area included
    // iroc,  oroc1, oroc2, oroc3, unrecognised (largest is chosen)
    Double_t xl_bp[] = {467.0, 595.8, 726.2, 867.0, 867.0};
    Double_t xs_bp[] = {292. , 457.8, 602.8, 733.3, 733.3};
    Double_t m_bp[]  = {496.5, 353.0, 350.0, 379.0, 379.0};
    Double_t xl = xl_bp[fType];
    Double_t xs = xs_bp[fType];
    Double_t m  = m_bp[fType];
    Double_t sx = fShift[0];
    Double_t sy = fShift[1];
    TLine * lf[4];
    // write as if the middle of the foil is in the origo, then shift it
    // long base
    lf[0] = new TLine(-xl/2.+sx, m/2.+sy, xl/2.+sx, m/2.+sy);
    // short base
    lf[1] = new TLine(-xs/2.+sx, -m/2.+sy, xs/2.+sx, -m/2.+sy);
    // left side
    lf[2] = new TLine(-xl/2.+sx, m/2.+sy, -xs/2.+sx, -m/2.+sy);
    // right side
    lf[3] = new TLine(xl/2.+sx, m/2.+sy, xs/2.+sx, -m/2.+sy);

    for(Int_t i=0;i<4;++i) lf[i]->Draw();
}

//----------------------------------------------------------------------------------
/*void MOpt::DrawProfiles(TPad * p, Int_t which_side, Int_t which_hole)
{
    p->cd();
    fhProfDiam[which_side][which_hole]->Draw("");
    ffProfFit[which_side][which_hole]->SetLineColor(kBlack);
    ffProfFit[which_side][which_hole]->SetLineStyle(2);
    ffProfFit[which_side][which_hole]->Draw("same");

    DrawFitResult(which_side, which_hole);

    p->Modified();
    p->Update();
}*/
//----------------------------------------------------------------------------------
void MOpt::DrawProfiles(TPad * p, Int_t which_side)
{
    p->cd();
    fhProfDiam[which_side][kRim]->SetLineColor(kRed+1);
    fhProfDiam[which_side][kInner]->SetLineColor(kGreen+1);
    fhProfDiam[which_side][kOuter]->SetLineColor(kBlue+1);
    ffProfFit[which_side][kRim]->SetLineColor(kRed+1);
    ffProfFit[which_side][kInner]->SetLineColor(kGreen+1);
    ffProfFit[which_side][kOuter]->SetLineColor(kBlue+1);

    TString fitopt = "";
    for(Int_t which_hole=2; which_hole>=0; --which_hole)
    {
        which_hole == 2 ? fitopt="" : fitopt="same";
        fhProfDiam[which_side][which_hole]->SetLineWidth(2);
        fhProfDiam[which_side][which_hole]->Draw(fitopt);
        ffProfFit[which_side][which_hole]->SetLineStyle(3);
        ffProfFit[which_side][which_hole]->Draw("same");
    }
    DrawFitResult(which_side);

    p->Modified();
    p->Update();
}
//----------------------------------------------------------------------------------
void MOpt::SaveTxt()
{
    if(!fIsLoaded[0] || !fIsLoaded[1])
    {
        std::cerr << "Load both datasets before saving\n";
    }
    else
    {
        SaveTxt1D();
        SaveTxt2D();
    }
}
void MOpt::SaveTxt2D()
{
    const Int_t nx = fhMapDiam[0][0]->GetNbinsX();
    const Int_t ny = fhMapDiam[0][0]->GetNbinsY();

    Double_t x = 0;
    Double_t y = 0;

    Int_t n_i_s = 0;
    Int_t n_o_s = 0;
    Int_t n_i_u = 0;
    Int_t n_o_u = 0;

    Double_t d_i_s = 0;
    Double_t d_o_s = 0;
    Double_t d_i_u = 0;
    Double_t d_o_u = 0;

    Double_t std_i_s = 0;
    Double_t std_o_s = 0;
    Double_t std_i_u = 0;
    Double_t std_o_u = 0;

    Double_t rim_s = 0;
    Double_t rim_u = 0;

    TString outfilename;
    //for(Int_t iside=0; iside<2; iside++)
    {
		outfilename = Form("%s/%s_2D.txt",fOutDir[0].Data(), fName.Data());
        std::cout << "Saving map to: " << outfilename << std::endl;
        std::ofstream ofs (outfilename.Data(), std::ofstream::out);

        for(Int_t ix=1; ix<=nx; ix++)
        {
            for(Int_t iy=1; iy<=ny; iy++)
            {
                //
                x = fhMapN[0][0]->GetXaxis()->GetBinCenter(ix) - fShift[0];
                y = fhMapN[0][0]->GetYaxis()->GetBinCenter(iy) - fShift[1];

                n_i_s = fhMapN[kSegmented][kInner]->GetBinContent(ix, iy);
                n_o_s = fhMapN[kSegmented][kOuter]->GetBinContent(ix, iy);
                n_i_u = fhMapN[kUnsegmented][kInner]->GetBinContent(ix, iy);
                n_o_u = fhMapN[kUnsegmented][kOuter]->GetBinContent(ix, iy);

                d_i_s = fhMapDiam[kSegmented][kInner]->GetBinContent(ix, iy);
                d_o_s = fhMapDiam[kSegmented][kOuter]->GetBinContent(ix, iy);
                d_i_u = fhMapDiam[kUnsegmented][kInner]->GetBinContent(ix, iy);
                d_o_u = fhMapDiam[kUnsegmented][kOuter]->GetBinContent(ix, iy);

                std_i_s = fhMapStd[kSegmented][kInner]->GetBinContent(ix, iy);
                std_o_s = fhMapStd[kSegmented][kOuter]->GetBinContent(ix, iy);
                std_i_u = fhMapStd[kUnsegmented][kInner]->GetBinContent(ix, iy);
                std_o_u = fhMapStd[kUnsegmented][kOuter]->GetBinContent(ix, iy);

                //rim_s = fhMapRim[kSegmented]->GetBinContent(ix, iy);
                //rim_u = fhMapRim[kUnsegmented]->GetBinContent(ix, iy);

                ofs << x <<"\t"<< y <<"\t"<< d_i_s << "\t" << d_o_s << "\t" << d_i_u << "\t" << d_o_u << "\t"
                                          << n_i_s << "\t" << n_o_s << "\t" << n_i_u << "\t" << n_o_u << "\t"
                                          << std_i_s << "\t" << std_o_s << "\t" << std_i_u << "\t" << std_o_u << "\t"
                                          << std::endl;
            }
            ofs << std::endl;
        }
        ofs.close();
    }
}

void MOpt::SaveTxt1D()
{
    const Int_t N = fhProfDiam[0][0]->GetNbinsX();
	const TString outfilename = Form("%s/%s_1D.txt",fOutDir[0].Data(), fName.Data());
    std::cout << "Saving profile to: " << outfilename << std::endl;
    std::ofstream ofs (outfilename.Data(), std::ofstream::out);

    Double_t x = 0;
    Double_t rs, ru, is, iu, os, ou; // rim seg., rim useg., inner seg., inner unseg., outer seg., outer unseg.
    for(Int_t ib=1; ib<=N; ib++)
    {
        x = fhProfDiam[0][0]->GetBinCenter(ib);
        rs = fhProfDiam[kSegmented][kRim]->GetBinContent(ib);
        ru = fhProfDiam[kUnsegmented][kRim]->GetBinContent(ib);
        is = fhProfDiam[kSegmented][kInner]->GetBinContent(ib);
        iu = fhProfDiam[kUnsegmented][kInner]->GetBinContent(ib);
        os = fhProfDiam[kSegmented][kOuter]->GetBinContent(ib);
        ou = fhProfDiam[kUnsegmented][kOuter]->GetBinContent(ib);

        ofs << x << "\t" << rs << "\t" << ru << "\t" << is << "\t" << iu << "\t" << os << "\t" << ou << std::endl;
    }
    ofs.close();
}

//----------------------------------------------------------------------------------
TString MOpt::GetSaveName()
{
	return Form("%s/Report_OPT_%s.pdf",fOutDir[0].Data(), fName.Data());
}

TString MOpt::GetROOTName(Int_t which_side)
{
    // don't append -s or -u if name contains it already
    TString side[] = {"-s", "-u"};
    if(fName.Contains("-s") || fName.Contains("-u")) return Form("%s/%s.root",fOutDir[which_side].Data(), fName.Data());
    else return Form("%s/%s%s.root",fOutDir[which_side].Data(), fName.Data(),side[which_side].Data());
}

#endif // MOPT_CXX
