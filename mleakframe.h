#include "mleak.cxx"

/// \class MLeakFrame
/// \brief Graphical Frame container for the leakage current measurement
///
/// A frame with 4 tabs (0: standard deviation of currents, 1: time-dependent currents, 2: first 10 min., 3: middle, 4: last 10 min. of measurement)
/// and buttons on the bottom responsible for loading a data set (currently only one is allowed), process it to automatically estimate the leakage current
/// along with the number of sparks. It also prints the results to a pdf file with name???.
/// The canvases are all clickable, upon clicking on a plot it brings up the corresponding figure zoommed.

class MLeakFrame
{
RQ_OBJECT("MLeakFrame")

private:
    Int_t               fIPlot; ///< Select what to draw on main canvas by clicking on tab
                                ///< (0: standard deviation of currents, 1: time-dependent currents, 2: first 10 min., 3: middle, 4: last 10 min.)
    Double_t            fxmin; ///< Left margin of the canvas pad (used in CreateDividedPad())
    Double_t            fymin; ///< Bottom margin of the canvas pad (used in CreateDividedPad())
    TGTransientFrame    * fMain; ///< Main frame
    TGStatusBar         * fStatusBar; ///< Status bar holding ???
    TGPictureButton     * load; ///< Buttom for opening file. Brings up pop-up window.
    TGPictureButton     * execute; ///< Button for processing the foils, estimates the leakage current, colors histograms according to that (red: above required leakage current, green: in required range)
    TGPictureButton     * save; ///< Button to save figures in a pdf file
    TGPictureButton     * clear; ///< Cleares loaded data
    TGPictureButton     * help;
    TGHorizontalFrame   * toolbar; ///< Container for buttons
    TGTab               * fTab; ///< Tab container
    TGCompositeFrame    * fCF[5]; ///< Container for canvases on each tab

protected:
    MLeak               * fLeak; ///< Custom class for GEM foil's leakage current measurement.
    TCanvas             * fCanv; ///< Transient canvas ???
    TRootEmbeddedCanvas * fEcanvasAll[5]; ///< Overview canvas for each tab
    TPad                * fPad[3][5];   ///< Further dividing fEcanvasAll[5]:
                                        ///< fPad[0][i] makes space for xlabel on all tabs
                                        ///< fPad[1][i] makes space for ylabel on all tabs
                                        ///< fPad[2][i] holds plots, [i] represents tabs
public:
    MLeakFrame(Int_t location, const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h); ///< Default constructor
    ~MLeakFrame(); ///< Default destructor

    //void DoExit();
    void Clear();
    void LoadCurrentFile();
    void ZoomFoil(Int_t event, Int_t px, Int_t py, TObject * obj);
    void ZoomFoilPrint(Int_t ich, Int_t itab, TString filename);

    void DoTab(Int_t);

    void AdjustPad(TPad * pad);
    void CreateDividedPad();

    void DrawCurrentTimeOverview();
    void DrawCurrentTimeOverviewZoom(Int_t itab);
    void DrawCurrentStdOverview();
    //void DrawCurrentZoomOverview();
    void ProcessFoilCurrents();

    void ClickOnPad(Int_t ich, Int_t &px, Int_t &py);
    Int_t ClickedOnPad(Int_t px, Int_t py);
    void EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();
    void CloseWindow();
};

/// \class MDialog
/// \brief Handles zoomed images which pop-up from MLeakFrame
///
/// Ensures the same formatting style

class MDialog
{
RQ_OBJECT("MDialog")

private:
    TGTransientFrame    * fMain; ///< Main frame

public:
    TCanvas             * fCanv; ///<

    MDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h); ///< default constructor
    ~MDialog(); ///< Default destructor
    TRootEmbeddedCanvas * fECanvasCh; ///< Main canvas for selected figure
    TGStatusBar         * fStatusBarLocal; ///< Mimic the main window's look with this
    void DrawCurrentStd(Int_t foil_id, MLeak * foil); ///< Draw standard deviation of leakage currents
    void DrawCurrentTime(Int_t itab, Int_t foil_id, MLeak * foil); ///< Draw time-dependence of currents
    //void DrawCurrentZoom(Int_t foil_id, MLeak * foil);
    void FoilInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();
    void CloseWindow();
};
