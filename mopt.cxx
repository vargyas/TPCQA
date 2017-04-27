
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
    // destructor
    if(fInFile[0]->IsOpen()) fInFile[0]->Close();
    if(fInFile[1]->IsOpen()) fInFile[1]->Close();
}
//----------------------------------------------------------------------------------
void MOpt::GuessFoilName(const TString name)
{
    fName = name;
    TString tmpname = name;

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
        fType = 4;
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
void MOpt::LoadFile(const TString filename, Int_t which_side)
{
    fInFileName[which_side] = filename;

    fDataDir[which_side] = gSystem->pwd();
    GuessFoilName(fDataDir[which_side]);
    GuessFoilType();

    // If h5 extension is selected, run converter to create ROOT file with TTree
    if( filename.Contains(".h5"))
    {
        // This program creates a ROOT tree from the optical scan's hdf5 file output
        const TString bash_command = Form(".! python ~/Dropbox/TPCQA/utils/loadHDF5tree.py %s",fDataDir[which_side].Data());
        std::cout << "Processing command:\n" << bash_command << std::endl;
        gROOT->ProcessLine(bash_command);

        fInFileName[which_side] = fDataDir[which_side]+"/outfile.root";
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

    // histogram sizes are the blueprint sizes in mm with the copper area included
    //               iroc,  oroc1, oroc2, oroc3, unrecognised (largest is chosen)
    Double_t xx[] = {467.0, 595.8, 726.2, 867.0, 867.0};
    Double_t yy[] = {496.5, 353.0, 350.0, 379.0, 379.0};
    Int_t nx[]    = {480,   480,   480,   480,   480};
    Int_t ny[]    = {320,   320,   320,   320,   320};

    //Double_t x_o[] = {60, 100};
    //Double_t x_i[] = {40, 80};
    //Double_t xbins = {0};

    for(Int_t j=0; j<2; j++)
    {
        // might need adjustment -yy[fType]+offset, offset
        fhMapDiam[i][j] = new TH2D(Form("hmap_diam_%d_%d",i,j),Form("%s diameter",holtyp[j].Data()),nx[fType],0,xx[fType],ny[fType],-yy[fType], 0);
        fhMapStd[i][j]  = new TH2D(Form("hmap_std_%d_%d",i,j),Form("std. of %s diameter",holtyp[j].Data()),nx[fType],0,xx[fType],ny[fType],-yy[fType], 0);
        fhMapN[i][j]    = new TH2D(Form("hmap_n_%d_%d",i,j),Form("N %s",holtyp[j].Data()),nx[fType],0,xx[fType],ny[fType],-yy[fType], 0);
    }
    fhProfDiam[i][kInner] = new TH1D(Form("hprof_diam_%d_%d",i,kInner),"",300,0,110);
    fhProfDiam[i][kOuter] = new TH1D(Form("hprof_diam_%d_%d",i,kOuter),"",300,0,110);
    fhProfDiam[i][kRim]   = new TH1D(Form("hprof_diam_%d_%d",i,kRim),"",300,0,110);
    for(Int_t j=0; j<3; j++)
    {
        fhProfDiam[i][j]->GetXaxis()->SetTitle("diameter [#mum]");
        fhProfDiam[i][j]->GetYaxis()->SetTitle("occurrence");
    }

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

    fhMapDiam[which_side][kInner]->Divide(fhMapN[which_side][kInner]);
    fhMapDiam[which_side][kInner]->GetZaxis()->SetRangeUser(40, 70);

    fhMapDiam[which_side][kOuter]->Divide(fhMapN[which_side][kOuter]);
    fhMapDiam[which_side][kOuter]->GetZaxis()->SetRangeUser(60, 100);

    fTree[which_side][kInner]->Draw(Form("d*4.34>>hprof_diam_%d_%d",which_side,kInner),"", "goff");
    fTree[which_side][kOuter]->Draw(Form("d*4.34>>hprof_diam_%d_%d",which_side,kOuter),"", "goff");
    //fTree[which_side][kRim]->Draw(Form("rim*4.34>>hprof_diam_%d_%d",which_side,kRim),"", "goff");

    fhProfDiam[which_side][kInner]->Fit(ffProfFit[which_side][kInner], "RNQ","",40,80);
    fhProfDiam[which_side][kOuter]->Fit(ffProfFit[which_side][kOuter], "RNQ","",60,100);

    CalculateStd(which_side);

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
void MOpt::CalculateStd(Int_t which_side)
{
    Double_t mean_i = ffProfFit[which_side][kInner]->GetParameter(1);
    Double_t mean_o = ffProfFit[which_side][kOuter]->GetParameter(1);
    Double_t std_i, std_o, inner, outer;
    std::cout << "Mean inner " << mean_i << "\nMean outer " << mean_o << std::endl;
    for(Int_t ibx=1; ibx<=fhMapStd[which_side][0]->GetNbinsX(); ++ibx)
    {
        for(Int_t iby=1; iby<=fhMapStd[which_side][0]->GetNbinsY(); ++iby)
        {
            inner = fhMapDiam[which_side][kInner]->GetBinContent(ibx, iby);
            outer = fhMapDiam[which_side][kOuter]->GetBinContent(ibx, iby);
            std_i = inner-mean_i;
            std_o = outer-mean_o;
            fhMapStd[which_side][kInner]->SetBinContent(ibx, iby, std_i);
            fhMapStd[which_side][kOuter]->SetBinContent(ibx, iby, std_o);
        }
    }
    //fhMapStd[which_side][kInner]->Divide(fhMapN[which_side][kInner]);
    //fhMapStd[which_side][kOuter]->Divide(fhMapN[which_side][kOuter]);
    fhMapStd[which_side][kInner]->GetZaxis()->SetRangeUser(-20, 20);
    fhMapStd[which_side][kOuter]->GetZaxis()->SetRangeUser(-20, 20);

    std::cout << "Mean inner " << mean_i << "\nMean outer " << mean_o << std::endl;

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

    p->Modified();
    p->Update();
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

    Int_t n_i = 0;
    Int_t n_o = 0;
    Double_t x = 0;
    Double_t y = 0;
    Double_t d_i = 0;
    Double_t d_o = 0;

    TString outfilename;
    TString side[] = {"seg", "unseg"};
    for(Int_t iside=0; iside<2; iside++)
    {
        outfilename = Form("%s/../%s_%s_2D.txt",fDataDir[0].Data(), fName.Data(), side[iside].Data());
        std::cout << "Saving diameter map to: " << outfilename << std::endl;
        std::ofstream ofs (outfilename.Data(), std::ofstream::out);

        for(Int_t ix=1; ix<=nx; ix++)
        {
            for(Int_t iy=1; iy<=ny; iy++)
            {
                n_i = fhMapN[iside][0]->GetBinContent(ix, iy);
                n_o = fhMapN[iside][1]->GetBinContent(ix, iy);

                d_i = fhMapDiam[iside][0]->GetBinContent(ix, iy);
                d_o = fhMapDiam[iside][1]->GetBinContent(ix, iy);

                ofs << ix << "\t" << iy  << "\t" << d_i << "\t" << d_o << "\t" << n_i << "\t" << n_o << std::endl;
            }
        }
        ofs.close();
    }
}

void MOpt::SaveTxt1D()
{
    const Int_t N = fhProfDiam[0][0]->GetNbinsX();
    const TString outfilename = Form("%s/../%s_1D.txt",fDataDir[0].Data(), fName.Data());
    std::cout << "Saving diameter profile to: " << outfilename << std::endl;
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
    return Form("%s/../Report_OPT_%s.pdf",fDataDir[0].Data(), fName.Data());
}
#endif // MOPT_CXX
