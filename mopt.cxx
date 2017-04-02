
#ifndef MOPT_CXX
#define MOPT_CXX

#include "mopt.h"
#include <sys/stat.h>
//#include <fstream>

bool FileExists(char* filename)
{
	struct stat fileInfo;
	return stat(filename, &fileInfo) == 0;
}

//----------------------------------------------------------------------------------
MOpt::MOpt()
{
	for(Int_t i=0; i<2; i++)
	{
		fIsLoaded[i]=kFALSE;
		fInFileName[i]="";
	}
}

//----------------------------------------------------------------------------------
MOpt::~MOpt()
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

    fName.ReplaceAll("data_","");
    fName.ReplaceAll("-s-1","");
    fName.ReplaceAll("-u-1","");
    fName.ReplaceAll("/","");

    std::cout << "guessed name of foil is: " << fName << std::endl;
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

    // If file it is a h5 file, run converter
    if( filename.Contains(".h5"))
	{
		// This program creates a ROOT tree from the optical scan's hdf5 file output
		const TString bash_command = Form(".! python ~/Dropbox/TPC_LeakageCurrent/utils/loadHDF5tree.py %s",fDataDir[which_side].Data());
		gROOT->ProcessLine(bash_command);

        fInFileName[which_side] = fDataDir[which_side]+"/outfile.root";
	}

    fInFile[which_side] = TFile::Open(fInFileName[which_side]);
    fInFile[which_side]->ls();

    fTree[which_side][kInner] = (TTree*) fInFile[which_side]->Get("inner_tree");
    fTree[which_side][kInner]->SetName(Form("inner_tree_%d", which_side));
    fTree[which_side][kOuter] = (TTree*) fInFile[which_side]->Get("outer_tree");
    fTree[which_side][kOuter]->SetName(Form("outer_tree_%d", which_side));

    CreateOutputContainers(which_side);

    FillOutputContainers(which_side);

	fIsLoaded[which_side] = kTRUE;
}
//----------------------------------------------------------------------------------
void MOpt::CloseFile(Int_t which_side)
{
    for(Int_t j=0; j<2; j++)
    {
        delete fhMapDiam[which_side][j];
		delete fhMapStd[which_side][j];
        delete fhMapN[which_side][j];
        delete fhProfDiam[which_side][j];
    }
    fInFile[which_side]->Close();
}

//----------------------------------------------------------------------------------
void MOpt::CreateOutputContainers(Int_t which_side)
{
    TString holtyp[] = {"inner", "outer"};
    Int_t i = which_side;

	Double_t x_o[] = {60, 100};
	Double_t x_i[] = {40, 80};

    for(Int_t j=0; j<2; j++)
    {
        if(fType==0)
        {
			fhMapDiam[i][j] = new TH2D(Form("hmap_diam_%d_%d",i,j),Form("%s diameter",holtyp[j].Data()),240,0,100000,160,-2560*43,2560*2);
			fhMapStd[i][j] = new TH2D(Form("hmap_std_%d_%d",i,j),Form("std. of %s diameter",holtyp[j].Data()),240,0,100000,160,-2560*43,2560*2);
			fhMapN[i][j]    = new TH2D(Form("hmap_n_%d_%d",i,j),Form("N %s",holtyp[j].Data()),240,0,100000,160,-2560*43,2560*2);
        }
        if(fType>0)
        {
			fhMapDiam[i][j] = new TH2D(Form("hmap_diam_%d_%d",i,j),Form("%s diameter",holtyp[j].Data()),240,0,88*1920,160,-33*2560,2*2560);
			fhMapStd[i][j] = new TH2D(Form("hmap_std_%d_%d",i,j),Form("std. of %s diameter",holtyp[j].Data()),240,0,88*1920,160,-33*2560,2*2560);
			fhMapN[i][j]    = new TH2D(Form("hmap_n_%d_%d",i,j),Form("N %s",holtyp[j].Data()),240,0,88*1920,160,-33*2560,2*2560);
        }
    }
	fhProfDiam[i][kInner] = new TH1D(Form("hprof_diam_%d_%d",i,kInner),"",300,40,110);
	fhProfDiam[i][kOuter] = new TH1D(Form("hprof_diam_%d_%d",i,kOuter),"",300,40,110);
	for(Int_t j=0; j<2; j++)
	{
		fhProfDiam[i][j]->GetXaxis()->SetTitle("diameter [#mum]");
		fhProfDiam[i][j]->GetYaxis()->SetTitle("occurrence");
	}

	ffProfFit[i][kInner] = new TF1(Form("inner_fit_%d", which_side),"gaus",40,110);
	ffProfFit[i][kOuter] = new TF1(Form("outer_fit_%d", which_side),"gaus",40,110);
}

//----------------------------------------------------------------------------------
void MOpt::FillOutputContainers(Int_t which_side)
{
	Double_t x_o[] = {60, 90};
	Double_t x_i[] = {40, 65};

	fTree[which_side][kInner]->Draw(Form("x:y>>hmap_diam_%d_%d",which_side,kInner),"diameter*4.35","goff");
	fTree[which_side][kOuter]->Draw(Form("x:y>>hmap_diam_%d_%d",which_side,kOuter),"diameter*4.35","goff");

	fTree[which_side][kInner]->Draw(Form("x:y>>hmap_std_%d_%d",which_side,kInner),"moment_2*4.35","goff");
	fTree[which_side][kOuter]->Draw(Form("x:y>>hmap_std_%d_%d",which_side,kOuter),"moment_2*4.35","goff");

	fTree[which_side][kInner]->Draw(Form("x:y>>hmap_n_%d_%d",which_side,kInner),"","goff");
    fTree[which_side][kOuter]->Draw(Form("x:y>>hmap_n_%d_%d",which_side,kOuter),"","goff");

    fhMapDiam[which_side][kInner]->Divide(fhMapN[which_side][kInner]);
	fhMapDiam[which_side][kInner]->GetZaxis()->SetRangeUser(x_i[0],x_i[1]);

    fhMapDiam[which_side][kOuter]->Divide(fhMapN[which_side][kOuter]);
	fhMapDiam[which_side][kOuter]->GetZaxis()->SetRangeUser(x_o[0],x_o[1]);

	//fhMapStd[which_side][kInner]->Divide(fhMapN[which_side][kInner]);
	//fhMapStd[which_side][kOuter]->Divide(fhMapN[which_side][kOuter]);
	fhMapStd[which_side][kInner]->GetZaxis()->SetRangeUser(0,10);
	fhMapStd[which_side][kOuter]->GetZaxis()->SetRangeUser(0,10);

	fhMapStd[which_side][kInner]->Print();
	fhMapStd[which_side][kOuter]->Print();
	// one also needs to set the range.

	fTree[which_side][kInner]->Draw(Form("diameter*4.4>>hprof_diam_%d_%d",which_side,kInner),"", "goff");
	fTree[which_side][kOuter]->Draw(Form("diameter*4.4>>hprof_diam_%d_%d",which_side,kOuter),"", "goff");

	fhProfDiam[which_side][kInner]->Fit(ffProfFit[which_side][kInner], "RNQ","",40,80);
	fhProfDiam[which_side][kOuter]->Fit(ffProfFit[which_side][kOuter], "RNQ","",60,100);
}
//----------------------------------------------------------------------------------
TString MOpt::GetFitResult(Int_t which_side, Int_t which_hole)
{
	TF1 * f = (TF1*) ffProfFit[which_side][which_hole];
	return Form("d = %.0f #pm %.1f #mum", f->GetParameter(1), f->GetParameter(2) );
}
TString MOpt::GetMeanWidth(Int_t which_side, Int_t which_hole)
{
	//if(which_hole==kInner) fhProfDiam[which_side][which_hole]->GetXaxis()->SetRange(40,70);
	//if(which_hole==kOuter) fhProfDiam[which_side][which_hole]->GetXaxis()->SetRange(70,110);
	// note that GetRMS() returns the standard deviation
	TString ret = Form("d = %.0f #pm %.1f #mum", fhProfDiam[which_side][which_hole]->GetMean(), fhProfDiam[which_side][which_hole]->GetRMS() );
	// restore original range
	//fhProfDiam[which_side][which_hole]->GetXaxis()->SetRange();
	return ret;
}

//----------------------------------------------------------------------------------
void MOpt::DrawFitResult(Int_t which_side)
{
	gROOT->SetStyle("Modern");

	TLegend * tinfo = new TLegend(0.6, 0.6, 0.89, 0.89, "", "brNDC");
	tinfo->SetFillStyle(0); tinfo->SetBorderSize(0); tinfo->SetTextSize(0.05);
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
	//TH1D * hDummyProfile = new TH1D("h","",100,40,100);
	//hDummyProfile->Draw();
	TString fitopt = "";
	for(Int_t which_hole=0; which_hole<2; which_hole++)
	{
		which_hole == 0 ? fitopt="" : fitopt="same";
		fhProfDiam[which_side][which_hole]->Draw(fitopt);
		ffProfFit[which_side][which_hole]->SetLineColor(kBlack);
		ffProfFit[which_side][which_hole]->SetLineStyle(2);
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

	for(Int_t iside=0; iside<2; iside++)
	{

		TString outfilename = Form("%s/outfile.txt", fDataDir[iside].Data());
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
	const TString outfilename = Form("%s/../%s.txt",fDataDir[0].Data(), fName.Data());
	std::cout << "Saving diameter profile to: " << outfilename << std::endl;
	std::ofstream ofs (outfilename.Data(), std::ofstream::out);

	Double_t x = 0;
	Double_t is, iu, os, ou; // inner segmented, inner unsegmented, outer segmented, outer unsegmented
	for(Int_t ib=1; ib<=N; ib++)
	{
		x = fhProfDiam[0][0]->GetBinCenter(ib);
		is = fhProfDiam[kSegmented][kInner]->GetBinContent(ib);
		iu = fhProfDiam[kUnsegmented][kInner]->GetBinContent(ib);
		os = fhProfDiam[kSegmented][kOuter]->GetBinContent(ib);
		ou = fhProfDiam[kUnsegmented][kOuter]->GetBinContent(ib);

		ofs << x << "\t" << is << "\t" << iu << "\t" << os << "\t" << ou << std::endl;
	}
	ofs.close();
}

//----------------------------------------------------------------------------------
TString MOpt::GetSaveName()
{
    return Form("%s/../Report_OPT_%s.pdf",fDataDir[0].Data(), fName.Data());
}
#endif // MOPT_CXX
