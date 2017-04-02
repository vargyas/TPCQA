#ifndef MGAIN_CXX
#define MGAIN_CXX

//----------------------------------------------------------------------------------
MGain::MGain()
{
	fMap[0] = new TH2D("hmeasurementmap","",240,0,240,160,0,160);
	fMap[1] = new TH2D("hreferencemap","",240,0,240,160,0,160);
	//fMap[2] = new TH2D("hgainmap","",240,0,240,160,0,160);
}
//----------------------------------------------------------------------------------
void MGain::LoadFile(TString infilename, Int_t i)
{
	// from file/dirname get foil number, set the reference measurement from
	// a run table? Now load both.
	TFile * fgain = TFile::Open(infilename, "READ");

	fInFile[i] = TFile::Open(infilename);
	fTree[i] = (TTree*) fInFile[i]->Get("gain_tree");

	ProcessFile();
}
//----------------------------------------------------------------------------------
// load data into histograms and calculate gain
void MGain::ProcessFile()
{
	TString hnames[] = {"hmeasurementmap", "hreferencemap", "hgainmap"};
	for(Int_t i=0; i<2; i++)
	{
		TString drawopt = Form("y:x>>%s",hnames[which].Data());
		fTree[which]->Draw(drawopt,"mean","N");
	}
	fMap[2] = (TH2D*) fMap[0]->Clone("hgainmap");
	fMap[2]->Divide(fMap[1]);
}
//----------------------------------------------------------------------------------

void MGain::DrawMap(TPad * p, Int_t which)
{
	p->cd();

	fMap[which]->Draw("colz");

	p->Modified();
	p->Update();
}

//----------------------------------------------------------------------------------

#endif // MGAIN_CXX
