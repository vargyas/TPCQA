#include "mfoil.cxx"

class MMainFrame
{
RQ_OBJECT("MMainFrame")

private:
    Int_t               fIPlot; ///< Select what to draw on main canvas by clicking on tab (0: standard deviation of currents, 1: time-dependent currents, 2: CH(i):CH(0) correlation)
    Double_t            fxmin, fymin;
    TGMainFrame         * fMain;
    TGStatusBar         * fStatusBar;
    TGPictureButton     * load, * execute, * save, * quit, * help;
    TGHorizontalFrame   * toolbar;
    TGTab               * fTab;
    TGCompositeFrame    * fCF[3];

protected:
    MFoil               * fFoil;
    TCanvas             * fCanv;
    //TPad              * fPad;
    TRootEmbeddedCanvas * fEcanvasAll[3];
    //TList             fPad;
    TPad                * fPad[3][3];   ///< fPad[0][i] makes space for xlabel on all tabs
                                        ///< fPad[1][i] makes space for ylabel on all tabs
                                        ///< fPad[2][i] holds plots, [i] represents tabs

public:
    MMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~MMainFrame();

    void DoExit();
    void LoadCurrentFile();
    void ZoomFoil(Int_t event, Int_t px, Int_t py, TObject * obj);
    void ZoomFoilPrint(Int_t ich, Int_t itab, TString filename);

    void DoTab(Int_t);

    void AdjustPad(TPad * pad);
    void CreateDividedPad();

    void DrawCurrentTimeOverview();
    void DrawCurrentStdOverview();
    void DrawCurrentCorrOverview();
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
