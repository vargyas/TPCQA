#include "moptframe.h"

void MOptFrame::AdjustPad(TPad * pad)
{
    pad->SetLeftMargin(0);pad->SetRightMargin(0);pad->SetTopMargin(0);pad->SetBottomMargin(0);
    pad->Range(0,0,100,100);
}

void MOptFrame::AdjustMapPad(TPad * pad)
{
    pad->SetLeftMargin(0.15);pad->SetRightMargin(0.1);pad->SetTopMargin(0.075);pad->SetBottomMargin(0.15);
    pad->Range(0,0,100,100);
}
void MOptFrame::CreateDividedPad2(Int_t itab)
{
    fXleft = 0.1;
    fXright = 0.002;
    fYbottom = 0.002;
    fYtop = 0.2;
    Double_t middle_y = fYbottom + (1.- fYtop - fYbottom)/2.;

    TPaveLabel * sides[2][4];

    fCanv = fEcanvasAll[itab]->GetCanvas();
    fCanv->cd();

    // create border tabs
    fPad[0][itab] = new TPad(Form("Pad_%d_%d",0,itab), "", 0.0, 0.0, 1.0, fYbottom); // bottom
    AdjustPad(fPad[0][itab]);
    fPad[1][itab] = new TPad(Form("Pad_%d_%d",1,itab), "", 0.0, fYbottom, fXleft, 1.0-fYtop); // left
    AdjustPad(fPad[1][itab]);
    fPad[2][itab] = new TPad(Form("Pad_%d_%d",2,itab), "", 1.0-fXright, fYbottom, 1.0, 1.0-fYtop); // right
    AdjustPad(fPad[2][itab]);
    fPad[3][itab] = new TPad(Form("Pad_%d_%d",3,itab), "", 0.0, 1.0-fYtop, 1.0, 1); // top
    AdjustPad(fPad[3][itab]);

    // create data tabs
    fPad[4][itab] = new TPad(Form("Pad_%d_%d",4,itab), "", fXleft, middle_y, 1-fXright, 1.0-fYtop); // top
    AdjustMapPad(fPad[4][itab]);
    fPad[5][itab] = new TPad(Form("Pad_%d_%d",5,itab), "", fXleft, fYbottom, 1-fXright, middle_y); // bottom
    AdjustMapPad(fPad[5][itab]);

    // Add segmented/unsegmented label
    sides[0][itab] = new TPaveLabel(0.0, 0.0, 100, 50, "Unsegmented");
    sides[0][itab]->SetBorderSize(0); sides[0][itab]->SetFillColor(kWhite);
    sides[0][itab]->SetTextSize(40./(fPad[1][itab]->GetBBox().fHeight));
    sides[0][itab]->SetTextAngle(90);
    sides[0][itab]->SetTextFont(42);

    sides[1][itab] = new TPaveLabel(0.0, 50, 100, 100, "Segmented");
    sides[1][itab]->SetBorderSize(0); sides[1][itab]->SetFillColor(kWhite);
    sides[1][itab]->SetTextSize(40./(fPad[1][itab]->GetBBox().fHeight));
    sides[1][itab]->SetTextAngle(90);
    sides[1][itab]->SetTextFont(42);

    for(Int_t i=0; i<6; i++)
    {
        fCanv->cd();
        fPad[i][itab]->Draw();
        fPad[i][itab]->cd();
        if(i==1)
        {
            sides[0][itab]->Draw();
            sides[1][itab]->Draw();
        }
    }
    fCanv->Modified();
    fCanv->Update();

}

void MOptFrame::CreateDividedPad4(Int_t itab)
{
    fXleft = 0.1;
    fXright = 0.002;
    fYbottom = 0.002;
    fYtop = 0.2;
    Double_t middle_x = fXleft + (1.- fXleft - fXright)/2.;
    Double_t middle_y = fYbottom + (1.- fYtop - fYbottom)/2.;

    TPaveLabel * sides[2][5];

    fCanv = fEcanvasAll[itab]->GetCanvas();
    fCanv->cd();

    // create border tabs
    fPad[0][itab] = new TPad(Form("Pad_%d_%d",0,itab), "", 0.0, 0.0, 1.0, fYbottom); // bottom
    AdjustPad(fPad[0][itab]);
    fPad[1][itab] = new TPad(Form("Pad_%d_%d",1,itab), "", 0.0, fYbottom, fXleft, 1.0-fYtop); // left
    AdjustPad(fPad[1][itab]);
    fPad[2][itab] = new TPad(Form("Pad_%d_%d",2,itab), "", 1.0-fXright, fYbottom, 1.0, 1.0-fYtop); // right
    AdjustPad(fPad[2][itab]);
    fPad[3][itab] = new TPad(Form("Pad_%d_%d",3,itab), "", 0.0, 1.0-fYtop, 1.0, 1); // top
    AdjustPad(fPad[3][itab]);

    // create data tabs
    fPad[4][itab] = new TPad(Form("Pad_%d_%d",4,itab), "", fXleft, middle_y, middle_x, 1.0-fYtop); // top left
    AdjustMapPad(fPad[4][itab]);
    fPad[5][itab] = new TPad(Form("Pad_%d_%d",5,itab), "", middle_x, middle_y, 1-fXright, 1.0-fYtop); // top right
    AdjustMapPad(fPad[5][itab]);
    fPad[6][itab] = new TPad(Form("Pad_%d_%d",6,itab), "", fXleft, fYbottom, middle_x, middle_y); // bottom left
    AdjustMapPad(fPad[6][itab]);
    fPad[7][itab] = new TPad(Form("Pad_%d_%d",7,itab), "", middle_x, fYbottom, 1-fXright, middle_y); // bottom right
    AdjustMapPad(fPad[7][itab]);

    // Add segmented/unsegmented label
    sides[0][itab] = new TPaveLabel(0.0, 0.0, 100, 50, "Unsegmented");
    sides[0][itab]->SetBorderSize(0); sides[0][itab]->SetFillColor(kWhite);
    sides[0][itab]->SetTextSize(40./(fPad[1][itab]->GetBBox().fHeight));
    sides[0][itab]->SetTextAngle(90);
    sides[0][itab]->SetTextFont(42);

    sides[1][itab] = new TPaveLabel(0.0, 50, 100, 100, "Segmented");
    sides[1][itab]->SetBorderSize(0); sides[1][itab]->SetFillColor(kWhite);
    sides[1][itab]->SetTextSize(40./(fPad[1][itab]->GetBBox().fHeight));
    sides[1][itab]->SetTextAngle(90);
    sides[1][itab]->SetTextFont(42);

    for(Int_t i=0; i<8; i++)
    {
        fCanv->cd();
        fPad[i][itab]->Draw();
        fPad[i][itab]->cd();
        if(i==1)
        {
            sides[0][itab]->Draw();
            sides[1][itab]->Draw();
        }
    }
    fCanv->Modified();
    fCanv->Update();
}

void MOptFrame::DrawFoilNameLabel(Bool_t clear)
{
    TPaveLabel * lname;
    // Add foil name label
    if(!clear) lname = new TPaveLabel(0.0, 50, 100, 100, fOpt->GetFoilName());
    else lname = new TPaveLabel(0.0, 50, 100, 100, "");
    lname->SetBorderSize(0); lname->SetFillColor(kWhite);
    lname->SetTextSize(30./(fPad[3][0]->GetBBox().fHeight));
    lname->SetTextFont(42);

    for(Int_t itab=0; itab<5; itab++)
    {
        fCanv = fEcanvasAll[itab]->GetCanvas();
        fCanv->cd(); fPad[3][itab]->cd();
        lname->Draw();
    }
}


MOptFrame::MOptFrame(Int_t location, const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h)
{

    fLoadedSegmented = false;
    fLoadedUnsegmented = false;

    // Create foil object (make sure to clear it)
    fOpt = new MOpt(location);

    // Create main frame
    fMain = new TGTransientFrame(p,main,w,h);

    // Create tabs
    fTab = new TGTab(fMain, 300, 300);
    fTab->Connect("Selected(Int_t)", "MLeakFrame", this, "DoTab(Int_t)");
    fIPlot = 0;

    TGCompositeFrame *tf;

    tf= fTab->AddTab("d MAP");
    fCF[0] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[0] = new TRootEmbeddedCanvas("EcanvasAllopt0",fCF[0],w-20,h-120);
    tf->AddFrame(fCF[0], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    tf = fTab->AddTab("d PROFILE");
    fCF[1] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[1] = new TRootEmbeddedCanvas("EcanvasAllopt1",fCF[1],w-20,h-120);
    tf->AddFrame(fCF[1], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    tf = fTab->AddTab("std(d) MAP");
    fCF[2] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[2] = new TRootEmbeddedCanvas("EcanvasAllopt2",fCF[2],w-20,h-120);
    tf->AddFrame(fCF[2], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    tf= fTab->AddTab("n MAP");
    fCF[3] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[3] = new TRootEmbeddedCanvas("EcanvasAllopt3",fCF[3],w-20,h-120);
    tf->AddFrame(fCF[3], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    tf= fTab->AddTab("rim MAP");
    fCF[4] = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
    fEcanvasAll[4] = new TRootEmbeddedCanvas("EcanvasAllopt3",fCF[4],w-20,h-120);
    tf->AddFrame(fCF[4], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    // status bar
    fStatusBar = new TGStatusBar(fMain, 60, 10, kVerticalFrame);
    //Int_t parts[] = {30, 10, 15};
    //fStatusBar->SetParts(parts, sizeof(parts)/sizeof(parts[0]));
    fStatusBar->SetParts(3);
    fStatusBar->Draw3DCorner(kFALSE);

    // Create a horizontal frame widget with buttons
    toolbar = new TGHorizontalFrame(fMain,w,0);

    // Create buttons for toolbar: load, execute, save and quit on the bottom of the panel
    load = new TGPictureButton(toolbar, gClient->GetPicture("ed_open.png")); //fileopen.xpm branch_folder_s.xpm folder_t.xpm
    load->SetToolTipText ("Open foil leakage current", 400);
    load->Connect("Clicked()","MOptFrame",this,"LoadFile()");
    load->Resize(40, 40);
    toolbar->AddFrame(load, new TGLayoutHints(kLHintsCenterX));

    clear = new TGPictureButton(toolbar, gClient->GetPicture("ed_delete.png")); //interrupt.png, mb_stop_s.xpm,
    clear->SetToolTipText ("Clear loaded histograms", 400);
    clear->Connect("Clicked()","MOptFrame",this,"Clear()");
    clear->Resize(40, 40);
    toolbar->AddFrame(clear, new TGLayoutHints(kLHintsCenterX));

    save = new TGPictureButton(toolbar, gClient->GetPicture("ed_save.png"));
    save->SetToolTipText ("Save profile histogram", 400);
    save->Connect("Clicked()","MOptFrame",this,"Save()");
    save->Resize(40, 40);
    toolbar->AddFrame(save, new TGLayoutHints(kLHintsCenterX));

    print = new TGPictureButton(toolbar, gClient->GetPicture("ed_print.png"));
    print->SetToolTipText ("Print canvas to pdf", 400);
    print->Connect("Clicked()","MOptFrame",this,"Print()");
    print->Resize(40, 40);
    toolbar->AddFrame(print, new TGLayoutHints(kLHintsCenterX));

    help = new TGPictureButton(toolbar, gClient->GetPicture("ed_help.png")); //mb_question_s.xpm
    help->SetToolTipText ("Help", 400);
    help->Connect("Clicked()","MOptFrame",this,"Help()");
    help->Resize(40, 40);
    toolbar->AddFrame(help, new TGLayoutHints(kLHintsCenterX));

    // Add frames to main frame

    fCF[0]->AddFrame(fEcanvasAll[0], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fCF[1]->AddFrame(fEcanvasAll[1], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fCF[2]->AddFrame(fEcanvasAll[2], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fCF[3]->AddFrame(fEcanvasAll[3], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fCF[4]->AddFrame(fEcanvasAll[4], new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    fMain->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    fMain->AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX) );

    fMain->AddFrame(toolbar, new TGLayoutHints(kLHintsCenterX | kLHintsBottom ));



    CreateDividedPad4(0);
    CreateDividedPad2(1);
    CreateDividedPad4(2);
    CreateDividedPad4(3);
    CreateDividedPad4(4);

    fCanv = fEcanvasAll[0]->GetCanvas();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MOptFrame",this,
                                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");
    fCanv = fEcanvasAll[1]->GetCanvas();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MOptFrame",this,
                                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");
    fCanv = fEcanvasAll[2]->GetCanvas();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MOptFrame",this,
                                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");
    fCanv = fEcanvasAll[3]->GetCanvas();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MOptFrame",this,
                                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");
    fCanv = fEcanvasAll[4]->GetCanvas();
    fCanv->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MOptFrame",this,
                                   "EventInfo(Int_t,Int_t,Int_t,TObject*)");

    // Set a name to the main frame
    fMain->SetWindowName("OPTICAL SCAN");
    // Map all subwindows of main frame
    fMain->MapSubwindows();
    // Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());
    // Map main frame
    fMain->MapWindow();
    fMain->SetCleanup(kDeepCleanup);
}

void MOptFrame::Help()
{
    std::cout << "help is not implemented yet, but this should bring up run table\n";
}

void MOptFrame::DoTab(Int_t itab)
{
    fIPlot=itab;
}

MOptFrame::~MOptFrame()
{
    // Delete all created widgets.
    fMain->Cleanup();
    delete fMain;
}

void MOptFrame::DrawMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[0]->GetCanvas();
    fCanv->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        fOpt->DrawMaps(fPad[4][0], kSegmented, kInner, 0);
        fOpt->DrawMaps(fPad[5][0], kSegmented, kOuter, 0);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        fOpt->DrawMaps(fPad[6][0], kUnsegmented, kInner, 0);
        fOpt->DrawMaps(fPad[7][0], kUnsegmented, kOuter, 0);
    }
}
void MOptFrame::DrawProfiles(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[1]->GetCanvas();
    fCanv->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        //fPad[4][1]->cd();
        fOpt->DrawProfiles(fPad[4][1],kSegmented);
        //fPad[5][1]->cd();
        //fOpt->DrawProfiles(fPad[5][1],kSegmented);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        //fPad[6][1]->cd();
        fOpt->DrawProfiles(fPad[5][1], kUnsegmented);
        //fPad[7][1]->cd();
        //fOpt->DrawProfiles(fPad[7][1], kUnsegmented, kOuter);
    }
}
void MOptFrame::DrawStdMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[2]->GetCanvas();
    fCanv->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        fOpt->DrawMaps(fPad[4][2], kSegmented, kInner, 1);
        fOpt->DrawMaps(fPad[5][2], kSegmented, kOuter, 1);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        fOpt->DrawMaps(fPad[6][2], kUnsegmented, kInner, 1);
        fOpt->DrawMaps(fPad[7][2], kUnsegmented, kOuter, 1);
    }
}
void MOptFrame::DrawDensityMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[3]->GetCanvas();
    fCanv->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        fOpt->DrawMaps(fPad[4][3], kSegmented, kInner, 2);
        fOpt->DrawMaps(fPad[5][3], kSegmented, kOuter, 2);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        fOpt->DrawMaps(fPad[6][3], kUnsegmented, kInner, 2);
        fOpt->DrawMaps(fPad[7][3], kUnsegmented, kOuter, 2);
    }
}
void MOptFrame::DrawRimMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv = fEcanvasAll[4]->GetCanvas();
    fCanv->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        fOpt->DrawMaps(fPad[4][4], kSegmented, kInner, 3);
        //fOpt->DrawMaps(fPad[5][4], kSegmented, kOuter, 3);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        fOpt->DrawMaps(fPad[6][4], kUnsegmented, kInner, 3);
        //fOpt->DrawMaps(fPad[7][4], kUnsegmented, kOuter, 3);
    }
}
void MOptFrame::EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected)
{
// Writes the event status in the status bar parts
   const char *text;
   char text2[50];
   //text = selected->GetName();
   fStatusBar->SetText(fOpt->GetFoilName(),0);
   if (event == kKeyPress)
      sprintf(text2, "%c", (char) px);
   else
      sprintf(text2, "%d,%d", px, py);
   fStatusBar->SetText(text2,1);
   text = selected->GetObjectInfo(px,py);
   fStatusBar->SetText(text,2);
   //std::cout << gPad->GetSelected()->GetName() << endl;
}

void MOptFrame::LoadFile()
{
    static  TString dir("~/cernbox/Work/ALICE/serviceWork/OS");
    TGFileInfo fi;
    fi.fFileTypes = filetypes_opt;
    fi.fIniDir    = StrDup(dir);
    new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);

    if(fi.fFilename)
    {
        printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
        dir = fi.fIniDir;

        Int_t which_side=-1;
        TString tmpname(fi.fFilename);

        if(tmpname.Contains("-s-") || tmpname.Contains("_S_") || tmpname.Contains("_S-")) {
            which_side = kSegmented;
            fLoadedSegmented = true;
        }
        else if(tmpname.Contains("-u-") || tmpname.Contains("_U_") || tmpname.Contains("_U-")) {
            which_side = kUnsegmented;
            fLoadedUnsegmented = true;
        }
        else {
            std::cerr << "Incorrect filename, expected -s-, _S_, _S-, -u-, _U_, _U-. Segmented side is assumed by default...\n";
            which_side = kSegmented;
        }

        std::cout << "loading file of side: " << which_side << std::endl;
        fOpt->LoadFile(fi.fFilename, which_side);

        DrawFoilNameLabel(false);

        DrawMaps(which_side);
        DrawProfiles(which_side);
        DrawStdMaps(which_side);
        DrawDensityMaps(which_side);
        DrawRimMaps(which_side);
    }
}

void MOptFrame::Save()
{
    fOpt->SaveTxt();
}

void MOptFrame::Print()
{
    gROOT->SetBatch(kTRUE);

    TString tmpname = fOpt->GetSaveName();
    std::cout << "Saving report to: " << tmpname << std::endl;
    /*
    fCanv = fEcanvasAll[0]->GetCanvas();
    fCanv->Print(Form("%s[",tmpname.Data()));
    fCanv->Print(tmpname);
    fCanv = fEcanvasAll[1]->GetCanvas();
    fCanv->Print(tmpname);
    fCanv = fEcanvasAll[3]->GetCanvas();
    fCanv->Print(tmpname);
    fCanv = fEcanvasAll[4]->GetCanvas();
    fCanv->Print(tmpname);
    fCanv->Print(Form("%s]",tmpname.Data()));
    */

    // maps are too big, saving to png first, then merge to pdf
    for(Int_t itab=0;itab<5;++itab)
    {
        fCanv = fEcanvasAll[itab]->GetCanvas();
        fCanv->Print(Form("opt%d.png",itab));
    }
    TString command = Form(".! convert opt*.png %s", tmpname.Data());
    std::cout << "Executing command: " << command << std::endl;
    gROOT->ProcessLine(command);
    // clean up
    //for(Int_t itab=0;itab<5;++itab)
        gROOT->ProcessLine( "remove( \"opt*.png\");" );

    gROOT->SetBatch(kFALSE);
}

void MOptFrame::Clear()
{
    if(fLoadedSegmented)   fOpt->CloseFile(kSegmented);
    if(fLoadedUnsegmented) fOpt->CloseFile(kUnsegmented);

    for(Int_t itab=0; itab<2; itab++)
    {
        fCanv = fEcanvasAll[itab]->GetCanvas();
        fCanv->cd();
        DrawFoilNameLabel(true);
        fCanv->Modified();
        fCanv->Update();
    }

}
