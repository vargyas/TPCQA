/**************************************************************************************
** Generates report of a GEM foil from segmented file name, assuming unsegmented file
** to have the same name (s->u). Actually a simplification of optical GUI MOptFrame,
** that is able to generate reports in batch mode, using only basic ROOT stuff, no GUI.
** Currently only handles ROOT file input, so run utils/runallh5.py before this.
**
** And make sure to compile it with (from the ROOT prompt):
** root [0] .L optreport.cxx++
** Then from bash:
**   > g++ -o optreport optreport.cxx `root-config --cflags --glibs` \
**         -lTreePlayer ./optreport_cxx.so -DSTANDALONE
** (Note that the TreePlayer has to be added as root-config does not handle it.)
** Then simply:
**   > ./optreport I-G2-013-s.root
**************************************************************************************/

#include "mopt.cxx"

class MOptFrame
{
private:
    Int_t fIPlot;
    Double_t fXleft,fXright, fYbottom, fYtop;
    bool fLoadedSegmented;
    bool fLoadedUnsegmented;

protected:
    MOpt                * fOpt;
    TCanvas             * fCanv[7];
    TPad                * fPad[10][7]; ///< max. 10 pads make space for labels, on 7 tabs

public:
    MOptFrame(Int_t location, UInt_t w, UInt_t h);
    virtual ~MOptFrame();
    void LoadROOTFile();
    void DrawFoilNameLabel(Bool_t clear);
    void AdjustPad(TPad * pad);
    void AdjustMapPad(TPad * pad);
    void AdjustErrorMapPad(TPad *pad);
    void CreateDividedPad2(Int_t itab);
    void CreateDividedPad4(Int_t itab);
    void CreateDividedPad6(Int_t itab);
    void LoadFileProtoScript(const TString infilename);

    void DrawMaps(Int_t which_side);
    void DrawStdMaps(Int_t which_side);
    void DrawProfiles(Int_t which_side);
    void DrawDensityMaps(Int_t which_side);
    void DrawRimMaps(Int_t which_side);
    void DrawErrorMaps(Int_t which_side);
    void DrawEccMaps(Int_t which_side);

    void Save();
    void Print();
};
//---------------------------------------------------------------------------
void MOptFrame::AdjustPad(TPad * pad)
{
    pad->SetLeftMargin(0);pad->SetRightMargin(0);
    pad->SetTopMargin(0);pad->SetBottomMargin(0);
    pad->Range(0,0,100,100);
}

//---------------------------------------------------------------------------
void MOptFrame::AdjustMapPad(TPad * pad)
{
    pad->SetLeftMargin(0.15);pad->SetRightMargin(0.1);
    pad->SetTopMargin(0.075);pad->SetBottomMargin(0.15);
    pad->Range(0,0,100,100);
}
//---------------------------------------------------------------------------
void MOptFrame::AdjustErrorMapPad(TPad * pad)
{
    pad->SetLeftMargin(0.05);pad->SetRightMargin(0.05);
    pad->SetTopMargin(0.075);pad->SetBottomMargin(0.15);
    pad->Range(0,0,100,100);
}
//---------------------------------------------------------------------------
void MOptFrame::CreateDividedPad2(Int_t itab)
{
    fXleft = 0.1;
    fXright = 0.002;
    fYbottom = 0.002;
    fYtop = 0.2;
    Double_t middle_y = fYbottom + (1.- fYtop - fYbottom)/2.;

    TPaveLabel * sides[2][4];

    fCanv[itab]->cd();

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
        fCanv[itab]->cd();
        fPad[i][itab]->Draw();
        fPad[i][itab]->cd();
        if(i==1)
        {
            sides[0][itab]->Draw();
            sides[1][itab]->Draw();
        }
    }
    fCanv[itab]->Modified();
    fCanv[itab]->Update();
}
//---------------------------------------------------------------------------
void MOptFrame::CreateDividedPad4(Int_t itab)
{
    fXleft = 0.1;
    fXright = 0.002;
    fYbottom = 0.002;
    fYtop = 0.2;
    Double_t middle_x = fXleft + (1.- fXleft - fXright)/2.;
    Double_t middle_y = fYbottom + (1.- fYtop - fYbottom)/2.;

    TPaveLabel * sides[2][6];

    fCanv[itab]->cd();

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
        fCanv[itab]->cd();
        fPad[i][itab]->Draw();
        fPad[i][itab]->cd();
        if(i==1)
        {
            sides[0][itab]->Draw();
            sides[1][itab]->Draw();
        }
    }
    fCanv[itab]->Modified();
    fCanv[itab]->Update();
}
//---------------------------------------------------------------------------
void MOptFrame::CreateDividedPad6(Int_t itab)
{
    fXleft = 0.1;
    fXright = 0.002;
    fYbottom = 0.002;
    fYtop = 0.2;
    Double_t middle_x_1 = fXleft + (1.- fXleft - fXright)/3.;
    Double_t middle_x_2 = fXleft + 2.*(1.- fXleft - fXright)/3.;
    Double_t middle_y = fYbottom + (1.- fYtop - fYbottom)/2.;

    TPaveLabel * sides[2][5];

    fCanv[itab]->cd();

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
    fPad[4][itab] = new TPad(Form("Pad_%d_%d",4,itab), "", fXleft, middle_y, middle_x_1, 1.0-fYtop); // top left
    AdjustErrorMapPad(fPad[4][itab]);
    fPad[5][itab] = new TPad(Form("Pad_%d_%d",5,itab), "", middle_x_1, middle_y, middle_x_2, 1.0-fYtop); // top middle
    AdjustErrorMapPad(fPad[5][itab]);
    fPad[6][itab] = new TPad(Form("Pad_%d_%d",6,itab), "", middle_x_2, middle_y, 1-fXright, 1.0-fYtop); // top right
    AdjustErrorMapPad(fPad[6][itab]);

    fPad[7][itab] = new TPad(Form("Pad_%d_%d",7,itab), "", fXleft, fYbottom, middle_x_1, middle_y); // bottom left
    AdjustErrorMapPad(fPad[7][itab]);
    fPad[8][itab] = new TPad(Form("Pad_%d_%d",8,itab), "", middle_x_1, fYbottom, middle_x_2, middle_y); // bottom middle
    AdjustErrorMapPad(fPad[8][itab]);
    fPad[9][itab] = new TPad(Form("Pad_%d_%d",9,itab), "", middle_x_2, fYbottom, 1-fXright, middle_y); // bottom right
    AdjustErrorMapPad(fPad[9][itab]);

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

    for(Int_t i=0; i<10; i++)
    {
        fCanv[itab]->cd();
        fPad[i][itab]->Draw();
        fPad[i][itab]->cd();
        if(i==1)
        {
            sides[0][itab]->Draw();
            sides[1][itab]->Draw();
        }
    }
    fCanv[itab]->Modified();
    fCanv[itab]->Update();
}
//---------------------------------------------------------------------------
void MOptFrame::DrawFoilNameLabel(Bool_t clear)
{
    TPaveLabel * lname;
    // Add foil name label
    if(!clear) lname = new TPaveLabel(0.0, 50, 100, 100, fOpt->GetFoilName());
    else lname = new TPaveLabel(0.0, 50, 100, 100, "");
    lname->SetBorderSize(0); lname->SetFillColor(kWhite);
    lname->SetTextSize(30./(fPad[3][0]->GetBBox().fHeight));
    lname->SetTextFont(42);

    for(Int_t itab=0; itab<7; itab++)
    {
        fCanv[itab]->cd(); fPad[3][itab]->cd();
        lname->Draw();
    }
}
//---------------------------------------------------------------------------
void MOptFrame::LoadFileProtoScript(const TString infilename)
{
    // output directory for pdf report and processed ROOT files
    // (better to use absolute path)
    //static TString odir("/home/vargyas/cernbox/Work/ALICE/serviceWork/OS");
    static TString odir("/mnt/alicetpc/storage/opticalData/results_svm");
    Int_t which_side=-1;

    if(infilename.Contains("-s") || infilename.Contains("_S_") || infilename.Contains("_S-")) {
        which_side = kSegmented;
        fLoadedSegmented = true;
    }
    else if(infilename.Contains("-u") || infilename.Contains("_U_") || infilename.Contains("_U-")) {
        which_side = kUnsegmented;
        fLoadedUnsegmented = true;
    }
    else {
        std::cerr << "Incorrect filename, expected -s, _S_, _S-, -u, _U_, _U-. Segmented side is assumed by default...\n";
        which_side = kSegmented;
    }

    std::cout << "loading file of side: " << which_side << std::endl;
    std::cout << "Possible sides: " << kSegmented << "\t" << kUnsegmented << std::endl;
    fOpt->LoadFile(infilename, odir, which_side);

    DrawFoilNameLabel(false);


    DrawMaps(which_side);
    DrawProfiles(which_side);
    DrawStdMaps(which_side);
    DrawDensityMaps(which_side);
    DrawRimMaps(which_side);
    DrawErrorMaps(which_side);
    DrawEccMaps(which_side);
}
//---------------------------------------------------------------------------
void MOptFrame::Save()
{
    fOpt->SaveTxt();
}
//---------------------------------------------------------------------------
void MOptFrame::Print()
{
    gROOT->SetBatch(kTRUE);

    TString tmpname = fOpt->GetSaveName();
    std::cout << "Saving report to: " << tmpname << std::endl;

    // maps are too big, saving to png first, then merge to pdf
    for(Int_t itab=0;itab<7;++itab)
    {
        fCanv[itab]->Print(Form("~/opt%d.png",itab));
    }
    TString command = Form(".! convert ~/opt*.png %s", tmpname.Data());
    std::cout << "Executing command: " << command << std::endl;
    gROOT->ProcessLine(command);
    // clean up
    //gROOT->ProcessLine( ".! rm ~/opt*.png" );
    gROOT->SetBatch(kFALSE);
}
//---------------------------------------------------------------------------
MOptFrame::MOptFrame(Int_t location, UInt_t w, UInt_t h)
{

    fLoadedSegmented = false;
    fLoadedUnsegmented = false;

    // Create foil object (make sure to clear it)
    fOpt = new MOpt(location);
    for(Int_t itab=0; itab<7; ++itab)
        fCanv[itab] = new TCanvas(Form("Canvas_%d",itab),"",w-20,h-120);
    CreateDividedPad4(0);
    CreateDividedPad2(1);
    CreateDividedPad4(2);
    CreateDividedPad4(3);
    CreateDividedPad4(4);
    CreateDividedPad6(5);
    CreateDividedPad4(6);
}
//---------------------------------------------------------------------------
MOptFrame::~MOptFrame()
{
    // Delete all created widgets.
    std::cout << "Destroying MOptFrame()...\n";
    delete fOpt;
}
//---------------------------------------------------------------------------
void MOptFrame::DrawMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv[0]->cd();
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

    fCanv[1]->cd();
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
//---------------------------------------------------------------------------
void MOptFrame::DrawStdMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv[2]->cd();
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
//---------------------------------------------------------------------------
void MOptFrame::DrawDensityMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv[3]->cd();
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
//---------------------------------------------------------------------------
void MOptFrame::DrawRimMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv[4]->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        fOpt->DrawMaps(fPad[4][4], kSegmented, kInner, 3);
        fOpt->DrawMaps(fPad[5][4], kSegmented, kInner, 4);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        fOpt->DrawMaps(fPad[6][4], kUnsegmented, kInner, 3);
        fOpt->DrawMaps(fPad[7][4], kUnsegmented, kInner, 4);
    }
}
//---------------------------------------------------------------------------
void MOptFrame::DrawErrorMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv[5]->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        fOpt->DrawMaps(fPad[4][5], kSegmented, kDefect, 2);
        fOpt->DrawMaps(fPad[5][5], kSegmented, kBlocked, 2);
        fOpt->DrawMaps(fPad[6][5], kSegmented, kEtching, 2);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        fOpt->DrawMaps(fPad[7][5], kUnsegmented, kDefect, 2);
        fOpt->DrawMaps(fPad[8][5], kUnsegmented, kBlocked, 2);
        fOpt->DrawMaps(fPad[9][5], kUnsegmented, kEtching, 2);
    }
}
//---------------------------------------------------------------------------
void MOptFrame::DrawEccMaps(Int_t which_side)
{
    gStyle->SetOptStat(0);

    fCanv[6]->cd();
    // draw segmented side
    if(which_side==kSegmented)
    {
        fOpt->DrawMaps(fPad[4][6], kSegmented, kInner, 5);
        fOpt->DrawMaps(fPad[5][6], kSegmented, kOuter, 5);
    }
    // draw unsegmented side
    if(which_side==kUnsegmented)
    {
        fOpt->DrawMaps(fPad[6][6], kUnsegmented, kInner, 5);
        fOpt->DrawMaps(fPad[7][6], kUnsegmented, kOuter, 5);
    }
}
//---------------------------------------------------------------------------


//void optreport(const TString infile_seg)
int main(int argc, char **argv)
{

    TString infile_seg = argv[1];

    //const TString basedir = "/home/vargyas/cernbox/Work/ALICE/serviceWork/OS/";
    const TString basedir = "/mnt/alicetpc/storage/opticalData/results_svm/";
    //const TString basedir = "/home/vargyas/Downloads/";

    TString infile_s = Form("%s%s",basedir.Data(),infile_seg.Data());
    TString infile_u(infile_s);
    infile_u.ReplaceAll("-s","-u");

    MOptFrame * opt = new MOptFrame(1, 1200, 900);
    std::cout << "==============\n runopt::loading file: " << infile_s << std::endl;
    opt->LoadFileProtoScript(infile_s);
    std::cout << "==============\n runopt::loading file: " << infile_u << std::endl;
    opt->LoadFileProtoScript(infile_u);

    opt->Print();
    opt->Save();

}
