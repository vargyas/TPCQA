#include "mfoil.cxx"

class MMainFrame
{
RQ_OBJECT("MMainFrame")

private:
    TGMainFrame         * fMain;
    TGStatusBar         * fStatusBar;
    TCanvas             * fCanv;
    TGPictureButton     * load, * execute, * save, * quit, * help;
    TGHorizontalFrame   * toolbar;
    TGTab               * fTab;

protected:
    MFoil               * fFoil;

public:
    TRootEmbeddedCanvas * fEcanvasAll;

    MMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~MMainFrame();

    void DoExit();
    void LoadCurrentFile();
    void DrawFoilCurrent(Int_t event, Int_t px, Int_t py, TObject * sel);
    void DrawFoilCurrent(Int_t foil_id);

    void DrawFoilCurrents();
    void DrawStdDevs();
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
    TCanvas             * fCanv;

public:
    MDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
    virtual ~MDialog();
    TRootEmbeddedCanvas * fECanvasCh;
    TGStatusBar         * fStatusBarLocal;
    void DrawFoilCurrent(Int_t foil_id, MFoil * foil);
    void FoilInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();

    //ClassDef(MDialog, 1);
};
