#include "mmainframe.h"

void MMainFrame::AdjustPad(TPad * pad)
{
    pad->SetLeftMargin(0);pad->SetRightMargin(0);pad->SetTopMargin(0);pad->SetBottomMargin(0);
    pad->Range(0,0,100,100);
}

void MMainFrame::CreateDividedPad()
{
    TPaveLabel * xtitle[3];
    TPaveLabel * ytitle[3];
    TString xlabel[] = {"Leakage current [nA]", "time [s]", "I_{0} [nA]"};
    TString ylabel[] = {"Occurence", "Leakage current [nA]", "I_{i} [nA]"};

    for(Int_t itab=0; itab<3; itab++)
    {
        fCanv = fEcanvasAll[itab]->GetCanvas();
        fCanv->cd();

        fPad[0][itab] = new TPad(Form("Pad_%d_%d",0,itab), "", 0.0, 0.0, 1.0, fymin);
        AdjustPad(fPad[0][itab]);
        fPad[1][itab] = new TPad(Form("Pad_%d_%d",1,itab), "", 0.0, fymin, fxmin, 1.0);
        AdjustPad(fPad[1][itab]);
        fPad[2][itab] = new TPad(Form("Pad_%d_%d",2,itab), "", fxmin, fymin, 1.0, 1.0);
        AdjustPad(fPad[2][itab]);

        // Add x/y title
        xtitle[itab] = new TPaveLabel(0.0, 0.0, 100, 100, xlabel[itab]);
        xtitle[itab]->SetBorderSize(0); xtitle[itab]->SetFillColor(kWhite);
        xtitle[itab]->SetTextSize(20./(fPad[0][itab]->GetBBox().fHeight));
        xtitle[itab]->SetTextFont(42);

        ytitle[itab] = new TPaveLabel(0.0, 0.0, 100, 100, ylabel[itab]);
        ytitle[itab]->SetBorderSize(0); ytitle[itab]->SetFillColor(kWhite);
        ytitle[itab]->SetTextSize(20./(fPad[1][itab]->GetBBox().fHeight));
        ytitle[itab]->SetTextAngle(90);
        ytitle[itab]->SetTextFont(42);

        // Divide data tab
        Int_t type = fFoil->GetType();
        if(type==0 || type==4) fPad[2][itab]->Divide(3, 6, 0.0, 0.0); // IROC and for unidentified foil
        if(type>=1 && type<=3) fPad[2][itab]->Divide(4, 6, 0.0, 0.0); // OROC 1, 2, 3

        fCanv->cd(); fPad[0][itab]->Draw(); fPad[0][itab]->cd(); xtitle[itab]->Draw();
        fCanv->cd(); fPad[1][itab]->Draw(); fPad[1][itab]->cd(); ytitle[itab]->Draw();
        fCanv->cd(); fPad[2][itab]->Draw();

        fCanv->Modified();
        fCanv->Update();
    }
}

MMainFrame::MMainFrame(const TGWindow *p, UInt_t w, UInt_t h)
{

	std::cout << "Your system arch. is: " << gSystem->GetBuildArch() << std::endl;
	//CpuInfo_t info;
	//gSystem->GetCpuInfo(&info);
	//std::cout << "\t your processor is: " << info << std::endl;

    fxmin = 0.1;
    fymin = 0.1;

    // Create foil object (make sure to clear it)
    fFoil = new MFoil(); 

    // Create main frame
    fMain = new TGMainFrame(p,w,h);

    // Create tabs
    fTab = new TGTab(fMain, 300, 300);
    fTab->Connect("Selected(Int_t)", "MMainFrame", this, "DoTab(Int_t)");
    fIPlot = 0;

    TGCompositeFrame *tf;

    tf= fTab->AddTab("STD");
    fCF[0] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[0] = new TRootEmbeddedCanvas("EcanvasAll0",fCF[0],w-20,h-120);
    tf->AddFrame(fCF[0], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    tf = fTab->AddTab("TIME");
    fCF[1] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[1] = new TRootEmbeddedCanvas("EcanvasAll1",fCF[1],w-20,h-120);
    tf->AddFrame(fCF[1], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    tf = fTab->AddTab("CORR");
    fCF[2] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[2] = new TRootEmbeddedCanvas("EcanvasAll2",fCF[2],w-20,h-120);
    tf->AddFrame(fCF[2], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    // status bar
    fStatusBar = new TGStatusBar(fMain, 60, 10, kVerticalFrame);
    //Int_t parts[] = {30, 10, 15};
    //fStatusBar->SetParts(parts, sizeof(parts)/sizeof(parts[0]));
    fStatusBar->SetParts(3);
    fStatusBar->Draw3DCorner(kFALSE);

    for(Int_t itab=0; itab<3; itab++)
    {
        fCanv = fEcanvasAll[itab]->GetCanvas();
        fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MMainFrame",this,
                       "ZoomFoil(Int_t,Int_t,Int_t,TObject*)");
    }
    // Create a horizontal frame widget with buttons
    toolbar = new TGHorizontalFrame(fMain,w,0);


    // Create buttons for toolbar: load, execute, save and quit on the bottom of the panel
    load = new TGPictureButton(toolbar, gClient->GetPicture("ed_open.png"));
    load->SetToolTipText ("Open foil leakage current", 400);
    load->Connect("Clicked()","MMainFrame",this,"LoadCurrentFile()"); 
    load->Resize(40, 40);
    toolbar->AddFrame(load, new TGLayoutHints(kLHintsCenterX));

    execute = new TGPictureButton(toolbar, gClient->GetPicture("ed_execute.png"));
    execute->SetToolTipText ("Evaluate foil", 400);
    execute->Connect("Clicked()", "MMainFrame", this, "ProcessFoilCurrents()");
    execute->Resize(40, 40);
    toolbar->AddFrame(execute, new TGLayoutHints(kLHintsCenterX));

    save = new TGPictureButton(toolbar, gClient->GetPicture("ed_save.png"));
    save->SetToolTipText ("Save foil parameters to database", 400);
    save->Connect("Clicked()","MMainFrame",this,"Save()");
    save->Resize(40, 40);
    toolbar->AddFrame(save, new TGLayoutHints(kLHintsCenterX));

    quit = new TGPictureButton(toolbar, gClient->GetPicture("ed_quit.png"));
    quit->SetToolTipText ("Quit application", 400);
    quit->Connect("Clicked()","MMainFrame",this,"DoExit()");
    quit->Resize(40, 40);
    toolbar->AddFrame(quit, new TGLayoutHints(kLHintsCenterX));

    help = new TGPictureButton(toolbar, gClient->GetPicture("ed_help.png"));
    help->SetToolTipText ("Help", 400);
    help->Resize(40, 40);
    toolbar->AddFrame(help, new TGLayoutHints(kLHintsCenterX)); 
    
    // Add frames to main frame

    fCF[0]->AddFrame(fEcanvasAll[0], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fCF[1]->AddFrame(fEcanvasAll[1], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fCF[2]->AddFrame(fEcanvasAll[2], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    fMain->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));


    //fMain->AddFrame(fEcanvasAll, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    
    fMain->AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX) );

    fMain->AddFrame(toolbar, new TGLayoutHints(kLHintsCenterX | kLHintsBottom ));   

    // Set a name to the main frame
    fMain->SetWindowName("TPC GEM QA");
    // Map all subwindows of main frame
    fMain->MapSubwindows();
    // Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());
    // Map main frame
    fMain->MapWindow();
}

void MMainFrame::DoTab(Int_t itab)
{
    fIPlot=itab;
}

MMainFrame::~MMainFrame()
{
    // Delete all created widgets.
    //delete fMain;
    //delete fEcanvasAll;
    //delete fFoil;
    //delete toolbar;
    //delete load;
    //delete execute;
    //delete save;
    //delete quit;
    //delete help;
    fMain->Cleanup(); // should do everything above
    delete fMain;
}




void MMainFrame::DrawCurrentStdOverview()
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[0]->GetCanvas();
    fCanv->cd();

    for(int ich=0; ich<fFoil->GetNC(); ich++)
    {
        fPad[2][0]->cd(ich+1);
        //TPad * subpad = (TPad*)(fPad2[0]->GetPrimitive(Form("%s_%d",fPad2[0]->GetName(),ich+1)));
        //cout << "drawing pad " << ich << "\t BB=" << subpad->GetBBoxCenter().fX << "\t" <<  subpad->GetBBoxCenter().fY << endl;
        //subpad->cd();
        fFoil->DrawCurrentStd(ich, fPad[2][0], false);
    }

    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MMainFrame",this,
                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");

    fCanv->Modified();
    fCanv->Update();
}

void MMainFrame::DrawCurrentTimeOverview()
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[1]->GetCanvas();
    fCanv->cd();

    for(int ich=0; ich<fFoil->GetNC(); ich++)
    {
        fPad[2][1]->cd(ich+1);
        fFoil->DrawCurrentTime(ich, fPad[2][1], false);
    }
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MMainFrame",this,
                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");

    fCanv->Modified();
    fCanv->Update();
}

void MMainFrame::DrawCurrentCorrOverview()
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[2]->GetCanvas();
    fCanv->cd();

    for(int ich=0; ich<fFoil->GetNC(); ich++)
    {
        fPad[2][2]->cd(ich+1);
        fFoil->DrawCurrentCorr(ich, fPad[2][2], false);
    }
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MMainFrame",this,
                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");

    fCanv->Modified();
    fCanv->Update();
}

void MMainFrame::EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected)
{
// Writes the event status in the status bar parts
   const char *text;
   char text2[50];
   //text = selected->GetName();
   fStatusBar->SetText(fFoil->GetInFileName(),0);
   if (event == kKeyPress)
      sprintf(text2, "%c", (char) px);
   else
      sprintf(text2, "%d,%d", px, py);
   fStatusBar->SetText(text2,1);
   text = selected->GetObjectInfo(px,py);
   fStatusBar->SetText(text,2);
   //std::cout << gPad->GetSelected()->GetName() << endl;
}

// returns the pad on which the mouse have clicked
// px, py in pixel coordinates
Int_t MMainFrame::ClickedOnPad(Int_t px, Int_t py)
{
    if(py > 520) return -1; // TODO: fix these with parameters
    if(px < 91) return -1;  // TODO: fix these with parameters

    UInt_t h = 100;
    UInt_t w = 100;

    Double_t _px = fPad[2][fIPlot]->AbsPixeltoX(px);
    Double_t _py = fPad[2][fIPlot]->AbsPixeltoY(py);
    _py = 100 - _py;

    Int_t nrows, ncolumns;
    switch(fFoil->GetType())
    {
        case 0: // IROC
            ncolumns = 3; nrows = 6; break; 
        case 1: // OROC1
            ncolumns = 4; nrows = 6; break; 
        case 2: // OROC2
            ncolumns = 4; nrows = 6; break; 
        case 3: // OROC3 
            ncolumns = 4; nrows = 6; break;
        default:
            ncolumns = 3; nrows = 6; break;
    }

    Double_t h_p = h/Double_t(nrows);
    Double_t w_p = w/Double_t(ncolumns);
    
    Int_t i_x = Int_t(_px/(w_p));
    Int_t i_y = Int_t(_py/(h_p));
    
    Int_t nch = fFoil->GetNC();
    Int_t ret = i_y * ncolumns + i_x;
    // check if clicked on empty pad
    if(ret > nch || ret < 0) ret = -1;
    
    return ret;
}

// returns middle of pad (px,py) for a given channel ich
// (inverse of ClickedOnPad())
void MMainFrame::ClickOnPad(Int_t ich, Int_t &px, Int_t &py)
{   
    UInt_t h = fEcanvasAll[fIPlot]->GetHeight();
    UInt_t w = fEcanvasAll[fIPlot]->GetWidth();
    
    Int_t nrows, ncolumns;
    switch(fFoil->GetType())
    {
        case 0: // IROC
            ncolumns = 3; nrows = 6; break; 
        case 1: // OROC1
            ncolumns = 4; nrows = 6; break; 
        case 2: // OROC2
            ncolumns = 4; nrows = 6; break; 
        case 3: // OROC3 
            ncolumns = 4; nrows = 6; break;
        default:
            ncolumns = 3; nrows = 6; break;
    }
    Double_t stepx = w/Double_t(ncolumns);
    Double_t stepy = h/Double_t(nrows);
    
    px = Int_t(stepx/2.) + Int_t(stepx) * (ich%ncolumns);
    py = Int_t(stepy/2.) + Int_t(stepy) * (ich/ncolumns);
}

void MMainFrame::LoadCurrentFile()
{   
	static  TString dir("../data");
    TGFileInfo fi;
    fi.fFileTypes = filetypes;
    fi.fIniDir    = StrDup(dir);
    new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);

    if(fi.fFilename)
    {    
        printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
        dir = fi.fIniDir;
        fFoil->LoadFoilCurrents(fi.fFilename);

        CreateDividedPad();

        DrawCurrentStdOverview();
        DrawCurrentTimeOverview();
        DrawCurrentCorrOverview();
    }
}

void MMainFrame::ProcessFoilCurrents()
{
    if(fFoil->GetLoadedStatus())
    {
        fFoil->ProcessFoilCurrents();
        // re-draw to include processing results
        DrawCurrentStdOverview();
        DrawCurrentTimeOverview();
        DrawCurrentCorrOverview();
    }
}

void MMainFrame::ZoomFoil(Int_t event, Int_t px, Int_t py, TObject * obj)
{
    if(event!=11) return;
    if(!fFoil->GetLoadedStatus()) return;

    Int_t foil_id = ClickedOnPad(px, py);
    if(foil_id < 0) return;

    MDialog * mdia = new MDialog(gClient->GetRoot(),fMain, 600, 600);

    switch(fIPlot)
    {
        case 0: mdia->DrawCurrentStd(foil_id, fFoil); break;
        case 1: mdia->DrawCurrentTime(foil_id, fFoil); break;
        case 2: mdia->DrawCurrentCorr(foil_id, fFoil); break;
    }
}

void MMainFrame::ZoomFoilPrint(Int_t foil_id, Int_t itab, TString filename)
{
    if(!fFoil->GetLoadedStatus()) return;
    if(!fFoil->GetProcessedStatus()) return;

	TCanvas * c = new TCanvas("ctemp","",580,580);
	TPad * pad = new TPad("dialogpad","",0,0,1,1);
	pad->SetLeftMargin(0.15); pad->SetRightMargin(0); pad->SetBottomMargin(0.15); pad->SetTopMargin(0);
	pad->Draw();
	pad->cd();

    switch(itab)
    {
		case 0: fFoil->DrawCurrentStd(foil_id, pad, true); break;
		case 1: fFoil->DrawCurrentTime(foil_id, pad, true); break;
		case 2: fFoil->DrawCurrentCorr(foil_id, pad, true); break;
    }
	c->Print(filename);
	delete c;
}

void MMainFrame::Save()
{
    gROOT->SetBatch(kTRUE);

	Int_t N = fFoil->GetNC();
	Int_t iplot=0;
	cout << "save channels "  << N << endl;

    // save current overview
    fCanv = fEcanvasAll[0]->GetCanvas();
    fCanv->Print(Form("Report%.2d.png",iplot++));

	for(Int_t ich = 0; ich<N; ich++)
    {
        ZoomFoilPrint(ich, 0, Form("Report%.2d.png",iplot++));
    }
    // save std.dev overview
    fCanv = fEcanvasAll[1]->GetCanvas();
    fCanv->Print(Form("Report%.2d.png",iplot++));
	for(Int_t ich = 0; ich<N; ich++)
    {
        ZoomFoilPrint(ich, 1, Form("Report%.2d.png",iplot++));
    }
    // save corr overview
    fCanv = fEcanvasAll[2]->GetCanvas();
    fCanv->Print(Form("Report%.2d.png",iplot++));

    gROOT->SetBatch(kFALSE);

	// merge to report and clean up intermediate files
    gROOT->ProcessLine(".! convert Report*.png REPORT.pdf");
	for(Int_t i=0; i<iplot; i++)
		gROOT->ProcessLine( Form("remove( \"Report%.2d.png\");",i) );


}

void MMainFrame::DoExit()
{
   Int_t ret = 0;
   new TGMsgBox(gClient->GetRoot(), fMain,
                fMain->GetWindowName(), "Quit application?",
                kMBIconExclamation, kMBYes | kMBNo, &ret);
                //kMBIconStop, kMBYes | kMBNo, &ret);
                //kMBIconAsterisk, kMBYes | kMBNo, &ret);
                //kMBIconQuestion, kMBYes | kMBNo, &ret);
  
   //if (ret == kMBYes) return gApplication->Terminate(0);
    if (ret == kMBYes)
    {
        gApplication->Terminate();
    }

}














void MDialog::FoilInfo(Int_t event, Int_t px, Int_t py, TObject * selected)
{
    const char *text;
    char text2[50];
    //fStatusBarLocal->SetText(fFoil->GetInFileName(),0);
    sprintf(text2, "%d,%d", px, py);
    fStatusBarLocal->SetText(text2,1);
    text = selected->GetName();
    fStatusBarLocal->SetText(text,2);
}

void MDialog::DrawCurrentStd(Int_t foil_id, MFoil * foil)
{
    gStyle->SetOptStat(0);

    // Set a name to the main frame
    fMain->SetWindowName(Form("Channel %d",foil_id));
    
    fCanv = fECanvasCh->GetCanvas();
    fCanv->Clear();
    fCanv->cd();
    TPad * pad = new TPad("dialogpad","",0,0,1,1);
    pad->SetLeftMargin(0.15); pad->SetRightMargin(0); pad->SetBottomMargin(0.15); pad->SetTopMargin(0);
    pad->Draw();
    pad->cd();

    foil->DrawCurrentStd(foil_id, pad, true);

    fCanv->Modified();
    fCanv->Update();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MDialog",this,
                   "FoilInfo(Int_t,Int_t,Int_t,TObject*)");
}

void MDialog::DrawCurrentTime(Int_t foil_id, MFoil * foil)
{
    gStyle->SetOptStat(0);

    // Set a name to the main frame
    fMain->SetWindowName(Form("Channel %d",foil_id));

    fCanv = fECanvasCh->GetCanvas();
    fCanv->Clear();
    fCanv->cd();
    TPad * pad = new TPad("dialogpad","",0,0,1,1);
    pad->SetLeftMargin(0.15); pad->SetRightMargin(0); pad->SetBottomMargin(0.15); pad->SetTopMargin(0);
    pad->Draw();
    pad->cd();

    foil->DrawCurrentTime(foil_id, pad, true);

    fCanv->Modified();
    fCanv->Update();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MDialog",this,
                   "FoilInfo(Int_t,Int_t,Int_t,TObject*)");
}

void MDialog::DrawCurrentCorr(Int_t foil_id, MFoil * foil)
{
    gStyle->SetOptStat(0);

    // Set a name to the main frame
    fMain->SetWindowName(Form("Channel %d",foil_id));

    fCanv = fECanvasCh->GetCanvas();
    fCanv->Clear();
    fCanv->cd();

    TPad * pad = new TPad("dialogpad","",0,0,1,1);
    pad->SetLeftMargin(0.15); pad->SetRightMargin(0); pad->SetBottomMargin(0.15); pad->SetTopMargin(0);
    pad->Draw();
    pad->cd();

    foil->DrawCurrentCorr(foil_id, pad, true);

    fCanv->Modified();
    fCanv->Update();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MDialog",this,
                   "FoilInfo(Int_t,Int_t,Int_t,TObject*)");
}



MDialog::MDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h)
{
    fMain = new TGTransientFrame(p, main, w, h);
    // use hierarchical cleaning
    fMain->SetCleanup(kDeepCleanup);
    
    fECanvasCh = new TRootEmbeddedCanvas("EcanvasCh",fMain,580,580);
    
    // status bar
    fStatusBarLocal = new TGStatusBar(fMain, 60, 10, kVerticalFrame);
    fStatusBarLocal->SetParts(3);
    fStatusBarLocal->Draw3DCorner(kFALSE);

    fMain->AddFrame(fECanvasCh, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY) );
    fMain->AddFrame(fStatusBarLocal, new TGLayoutHints(kLHintsExpandX) );
    
    // Map all subwindows of main frame
    fMain->MapSubwindows();
    // Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());
    // Map main frame
    fMain->MapWindow();     
}

MDialog::~MDialog()
{
    fMain->DeleteWindow();
    //delete fMain;
    //delete fCanv;
    //delete fECanvasCh;
    //delete fStatusBarLocal;
}

