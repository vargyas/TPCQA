#include "mmainframe.h"


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
    //delete fMain;
    //delete fEcanvasAll;
    //delete fFoil;
    //delete toolbar;
    //delete load;
    //delete execute;
    //delete save;
    //delete quit;
    //delete help;
    fMain->Cleanup();
    delete fMain;
}

void MMainFrame::DrawStdDevs()
{
    Int_t type = fFoil->GetType();
    fCanv = fEcanvasAll->GetCanvas();
    fCanv->Clear();

    if(type==0 || type==4) fCanv->Divide(3, 6, 0, 0); // IROC and for unidentified foil
    if(type>=1 && type<=3) fCanv->Divide(4, 6, 0, 0); // OROC 1, 2, 3 

    TPad* cdiv[24];
    gStyle->SetOptStat(0);
    for(int ich=0; ich<fFoil->GetNC(); ich++)
    {
        cdiv[ich] = (TPad*) fCanv->GetListOfPrimitives()->FindObject(Form("EcanvasAll_%d",ich+1));
        cdiv[ich]->SetName(Form("CH%d",ich+1));
        cdiv[ich]->cd();
        fFoil->DrawStdDev(ich, "hist");     
    }
    fCanv->Modified();
    fCanv->Update();
}

void MMainFrame::DrawFoilCurrents()
{
    //std::cout << "DrawFoilCurrents \n";
    Int_t type = fFoil->GetType();
    fCanv = fEcanvasAll->GetCanvas();
    fCanv->Clear();

    if(type==0 || type==4) fCanv->Divide(3, 6, 0, 0); // IROC and for unidentified foil
    if(type>=1 && type<=3) fCanv->Divide(4, 6, 0, 0); // OROC 1, 2, 3 

    TPad* cdiv[24];
    gStyle->SetOptStat(0);
    for(int ich=0; ich<fFoil->GetNC(); ich++)
    {
        cdiv[ich] = (TPad*) fCanv->GetListOfPrimitives()->FindObject(Form("EcanvasAll_%d",ich+1));
        cdiv[ich]->SetName(Form("CH%d",ich+1));
        cdiv[ich]->cd();
        
        cdiv[ich]->SetLogy();
        fFoil->DrawHChannel(ich, "hist");
        fFoil->DrawHLimit();
                
        if(fFoil->GetProcessedStatus()) 
        {
            fFoil->DrawSatCurrent(ich);
            fFoil->DrawMeasurementRange(ich);
            fFoil->DrawSparks(ich,"hist same");
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
    
    Int_t i_x = Int_t(Double_t(px)/(w_p));
    Int_t i_y = Int_t(Double_t(py)/(h_p));
    
    Int_t nch = fFoil->GetNC();
    Int_t ret = i_y * ncolumns + i_x;
    // check if clicked on empty pad
    if(ret > nch) ret = -1;
    
    return ret;
}

// returns middle of pad (px,py) for a given channel ich
// (inverse of ClickedOnPad())
void MMainFrame::ClickOnPad(Int_t ich, Int_t &px, Int_t &py)
{   
    UInt_t h = fEcanvasAll->GetHeight();
    UInt_t w = fEcanvasAll->GetWidth();
    
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

void MMainFrame::DrawFoilCurrent(Int_t event, Int_t px, Int_t py, TObject * selected)
{
    if(event!=11) return;
    if(!fFoil->GetLoadedStatus()) return;

    Int_t foil_id = ClickedOnPad(px, py);
    if(foil_id < 0) return;
    std::cout << "channel id = " << foil_id << std::endl;

    MDialog * mdia = new MDialog(gClient->GetRoot(),fMain, 600, 600);
    mdia->DrawFoilCurrent(foil_id, fFoil);
}


void MMainFrame::DrawFoilCurrent(Int_t foil_id)
{
    fCanv = fEcanvasAll->GetCanvas();
    fCanv->Clear();
    fFoil->DrawHChannel(foil_id, "hist");
    fFoil->DrawHLimit();

    fCanv->SetLogy();
    gStyle->SetOptStat(0);

    if(fFoil->GetProcessedStatus())
    {
        fFoil->DrawSatCurrent(foil_id);
        fFoil->DrawSparks(foil_id,"hist same");
        fFoil->DrawMeasurementRange(foil_id);
    }
}

void MMainFrame::Save()
{
    gROOT->SetBatch(kTRUE);
    
    // draw current overview
    DrawFoilCurrents();
    fCanv->Print(Form("Report%s.pdf[",fFoil->GetName().Data()), "pdf");
    fCanv->Print(Form("Report%s.pdf",fFoil->GetName().Data()));
    
    // draw std.dev overview
    DrawStdDevs();
    fCanv->Print(Form("Report%s.pdf",fFoil->GetName().Data()));
    
    // draw each current per channel
    for(Int_t ich = 0; ich<fFoil->GetNC(); ++ich)
    {
        //ClickOnPad(ich, px, py); // artificial click on each pad, get px, py from that
        DrawFoilCurrent(ich);
        fCanv->Print(Form("Report%s.pdf",fFoil->GetName().Data()));
    }
    fCanv->Print(Form("Report%s.pdf]",fFoil->GetName().Data()));
    
    gROOT->SetBatch(kFALSE);
    DrawFoilCurrents();
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

void MDialog::DrawFoilCurrent(Int_t foil_id, MFoil * foil)
{
    // Set a name to the main frame
    fMain->SetWindowName(Form("Channel %d",foil_id+1));
    
    fCanv = fECanvasCh->GetCanvas();
    //fCanv = fEcanvasCh[foil_id]->GetCanvas();
    fCanv->Clear();
    foil->DrawHChannel(foil_id, "hist");
    foil->DrawHLimit();

    fCanv->SetLogy();
    gStyle->SetOptStat(0);

    if(foil->GetProcessedStatus())
    {
        foil->DrawSatCurrent(foil_id);
        foil->DrawSparks(foil_id,"hist same");
        foil->DrawMeasurementRange(foil_id);

        TLegend * tinfo = new TLegend(0.5, 0.5, 0.98, 0.8, "", "brNDC");
        tinfo->SetFillStyle(0); tinfo->SetBorderSize(0); tinfo->SetTextSize(0.05);
        tinfo->AddEntry((TObject*)0,"Stat", "");
        tinfo->AddEntry((TObject*)0,foil->GetInfoSatCurrent(foil_id), "");
        //tinfo->AddEntry((TObject*)0,foil->GetInfoNumSparks(foil_id),  "");
        tinfo->Draw();
    }
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

