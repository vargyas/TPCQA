#include "mfoil.cxx"

class MMainFrame
{
RQ_OBJECT("MMainFrame")

private:
    Int_t               fIPlot; ///< Select what to draw on main canvas by clicking on tab (0: standard deviation of currents, 1: time-dependent currents, 2: CH(i):CH(0) correlation)
    TGMainFrame         * fMain;
    TGStatusBar         * fStatusBar;

    TGPictureButton     * load, * execute, * save, * quit, * help;
    //TGPictureButton       * prev, * next;
    TGHorizontalFrame   * toolbar;
    TGTab               * fTab;
    TGCompositeFrame    * fCF[3];


protected:
    MFoil               * fFoil;

public:
    TCanvas             * fCanv;
    TRootEmbeddedCanvas * fEcanvasAll[3];

    MMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~MMainFrame();

    void DoExit();
    void LoadCurrentFile();
    void ZoomFoil(Int_t event, Int_t px, Int_t py);
    void DrawFoilCurrent(Int_t foil_id);

    void DoTab(Int_t);

    void DrawCurrentTimeOverview();
    void DrawCurrentStdOverview();
    void ProcessFoilCurrents();

    void ClickOnPad(Int_t ich, Int_t &px, Int_t &py);
    Int_t ClickedOnPad(Int_t px, Int_t py);
    void EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();

    //ClassDef(MMainFrame, 1);
};

class MDialog
{
RQ_OBJECT("MDialog")

private:
    TGTransientFrame    * fMain;

//protected:
//    MFoil               * fFoil;

public:
    TCanvas             * fCanv;

    MDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
    virtual ~MDialog();
    TRootEmbeddedCanvas * fECanvasCh;
    TGStatusBar         * fStatusBarLocal;
    void DrawCurrentStd(Int_t foil_id, MFoil * foil);
    void DrawCurrentTime(Int_t foil_id, MFoil * foil);
    void DrawCurrentCorr(Int_t foil_id, MFoil * foil);
    void FoilInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();

    //ClassDef(MDialog, 1);
};
