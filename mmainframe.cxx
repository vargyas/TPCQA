#include "mmainframe.h"
#include "mfoil.cxx"



MMainFrame::MMainFrame(const TGWindow *p, UInt_t w, UInt_t h)
{
	std::cout << "MMainFrame\n";
	
	// Create a few colors:
	Pixel_t yellow, red, green, blue, orange;
	gClient->GetColorByName("yellow",yellow);
	gClient->GetColorByName("red",red);
	gClient->GetColorByName("green",green);
	gClient->GetColorByName("blue",blue);
	gClient->GetColorByName("orange",orange);

	// Create foil object (make sure to clear it)
    fFoil = new MFoil(); 

	// Create main frame
    fMain = new TGMainFrame(p,w,h);

    // Create canvas widget	
	fEcanvasAll = new TRootEmbeddedCanvas("EcanvasAll",fMain,w-20,h-120);
	
	// status bar
    fStatusBar = new TGStatusBar(fMain, 60, 10, kVerticalFrame);
    //Int_t parts[] = {30, 10, 15};
	//fStatusBar->SetParts(parts, sizeof(parts)/sizeof(parts[0]));
	fStatusBar->SetParts(3);
    fStatusBar->Draw3DCorner(kFALSE);

	fCanv = fEcanvasAll->GetCanvas();
	fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MMainFrame",this,
                   "DrawFoilCurrent(Int_t,Int_t,Int_t,TObject*)");
	
    // Create a horizontal frame widget with buttons
    TGHorizontalFrame *toolbar = new TGHorizontalFrame(fMain,w,0);


	// Create buttons for toolbar: load, execute, save and quit on the bottom of the panel
    TGPictureButton * load = new TGPictureButton(toolbar, gClient->GetPicture("ed_open.png"));
    load->SetToolTipText ("Open foil leakage current", 400);
    load->Connect("Clicked()","MMainFrame",this,"LoadCurrentFile()"); // TODO: change Clicked, error when nothing selected...
	load->Resize(40, 40);
    toolbar->AddFrame(load, new TGLayoutHints(kLHintsCenterX));

    TGPictureButton * execute = new TGPictureButton(toolbar, gClient->GetPicture("ed_execute.png"));
    execute->SetToolTipText ("Evaluate foil", 400);
	execute->Connect("Clicked()", "MMainFrame", this, "ProcessFoilCurrents()");
	execute->Resize(40, 40);
    toolbar->AddFrame(execute, new TGLayoutHints(kLHintsCenterX));

    TGPictureButton * save = new TGPictureButton(toolbar, gClient->GetPicture("ed_save.png"));
    save->SetToolTipText ("Save foil parameters to database", 400);
	save->Resize(40, 40);
    toolbar->AddFrame(save, new TGLayoutHints(kLHintsCenterX));

    TGPictureButton * quit = new TGPictureButton(toolbar, gClient->GetPicture("ed_quit.png"));
    quit->SetToolTipText ("Quit application", 400);
    quit->Connect("Clicked()","MMainFrame",this,"DoExit()");
	quit->Resize(40, 40);
    toolbar->AddFrame(quit, new TGLayoutHints(kLHintsCenterX));

    TGPictureButton * help = new TGPictureButton(toolbar, gClient->GetPicture("ed_help.png"));
    help->SetToolTipText ("Help", 400);
	help->Resize(40, 40);
    toolbar->AddFrame(help, new TGLayoutHints(kLHintsCenterX));	
	
	//  Create a top bar to hold info and more elaborate commands
	//  TGHorizontalFrame *topbar = new TGHorizontalFrame(fMain,w,0);
	//  fMain->AddFrame(topbar, new TGLayoutHints(kLHintsCenterX | kLHintsTop  ));
	
	// Add frames to main frame
    

	fMain->AddFrame(fEcanvasAll, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
	
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

MMainFrame::~MMainFrame()
{
    // Delete all created widgets.
    delete fMain;
    delete fEcanvasAll;
	delete[] *fEcanvasCh;
    delete fFoil;
}

void MMainFrame::CloseWindow()
{
   // Got close message for this MainFrame. Terminates the application.

   //gApplication->Terminate();
}

void MMainFrame::Created()
{ 
    Emit("Created()"); 
} //*SIGNAL*
    
void MMainFrame::Welcome()
{ 
    printf("MMainFrame has been created. Welcome!\n"); 
}

void MMainFrame::DrawFoilCurrents()
{
	//std::cout << "DrawFoilCurrents \n";
	int N = fFoil->GetNC();
    fCanv = fEcanvasAll->GetCanvas();
	fCanv->Clear();

    if(N==18) fCanv->Divide(3, 6, 0, 0); // IROC
	if(N==24) fCanv->Divide(4, 6, 0, 0); // OROC

	TPad* cdiv[24];
	gStyle->SetOptStat(0);
    for(int ich=0; ich<N; ich++)
    {
        cdiv[ich] = (TPad*) fCanv->GetListOfPrimitives()->FindObject(Form("EcanvasAll_%d",ich+1));
		cdiv[ich]->SetName(Form("CH%d",ich+1));
        cdiv[ich]->cd();
        fFoil->DrawHChannel(ich, "hist");
		fFoil->DrawHLimit();
		
		cdiv[ich]->SetLogy();
		
		if(fFoil->GetProcessedStatus()) {
			//fFoil->DrawSatCurrent(ich);
			fFoil->DrawMeasurementRange(ich);
		
			TList *functions = ((TH1D*)fFoil->fhChannelPos.At(ich))->GetListOfFunctions();
			TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
			if(pm) pm->Draw("same");
		}
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
}

// returns the pad on which the mouse have clicked
Int_t MMainFrame::ClickedOnPad(Int_t px, Int_t py)
{
	UInt_t h = fEcanvasAll->GetHeight();
	UInt_t w = fEcanvasAll->GetWidth();
	
	Int_t nrows;
	fFoil->GetType()==0 ? nrows=3 : nrows=4; // OROC = 3*6, IROC = 4*6
	Double_t h_p = h/6.;
	Double_t w_p = w/Double_t(nrows);
	
	Int_t i_x = Int_t(Double_t(px)/(w_p));
	Int_t i_y = Int_t(Double_t(py)/(h_p));
	
	return i_y * nrows + i_x;
}

void MMainFrame::DrawFoilCurrent(Int_t event, Int_t px, Int_t py, TObject * selected)
{
	if(event!=11) return;
	Int_t foil_id = ClickedOnPad(px, py);
	fSide = new TGTransientFrame(gClient->GetRoot(), fMain, 600, 600);
	fEcanvasCh[foil_id] = new TRootEmbeddedCanvas(Form("EcanvasCh%d",foil_id),fSide,580,580);

	fCanv = fEcanvasCh[foil_id]->GetCanvas();
	fCanv->Clear();
	fFoil->DrawHChannel(foil_id,"hist");
	fFoil->DrawHLimit();

	fCanv->SetLogy();
	gStyle->SetOptStat(0);

	if(fFoil->GetProcessedStatus()) 
	{
		fFoil->DrawSatCurrent(foil_id);
		fFoil->DrawMeasurementRange(foil_id);
		
		TList *functions = ((TH1D*)fFoil->fhChannelPos.At(foil_id))->GetListOfFunctions();
		TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
		if(pm) pm->Draw("same");
			
		TLegend * tinfo = new TLegend(0.5, 0.5, 0.98, 0.8, "", "brNDC");
		tinfo->SetFillStyle(0); tinfo->SetBorderSize(0); tinfo->SetTextSize(0.05);
		tinfo->AddEntry((TObject*)0,"Stat", "");
		tinfo->AddEntry((TObject*)0,fFoil->GetInfoSatCurrent(foil_id), "");
		//tinfo->AddEntry((TObject*)0,fFoil->GetInfoNumSparks(foil_id), "");
		tinfo->Draw();
	}

	fSide->AddFrame( fEcanvasCh[foil_id], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY) );
    // Set a name to the main frame
    fSide->SetWindowName("TPC GEM QA");
    // Map all subwindows of main frame
    fSide->MapSubwindows();
    // Initialize the layout algorithm
    fSide->Resize(fSide->GetDefaultSize());
    // Map main frame
    fSide->MapWindow();	
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
		DrawFoilCurrents();
	}
}


void MMainFrame::ProcessFoilCurrents()
{
	if(fFoil->GetLoadedStatus())
	{
		fFoil->ProcessFoilCurrents();
		DrawFoilCurrents(); // re-draw to include processing results 
	}
}

void MMainFrame::DoExit()
{
   int ret = 0;

   new TGMsgBox(gClient->GetRoot(), fMain,
                fMain->GetWindowName(), "Quit application?",
                kMBIconExclamation, kMBYes | kMBNo, &ret);
                //kMBIconStop, kMBYes | kMBNo, &ret);
                //kMBIconAsterisk, kMBYes | kMBNo, &ret);
                //kMBIconQuestion, kMBYes | kMBNo, &ret);
  
   //if (ret == kMBYes) return gApplication->Terminate(0);
    if (ret == kMBYes) {
		fMain->CloseWindow();
		gApplication->Terminate();
	}

}


