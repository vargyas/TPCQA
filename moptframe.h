#ifndef MOPTFRAME_H
#define MOPTFRAME_H

#include "mopt.cxx"


class MOptFrame
{
RQ_OBJECT("MOptFrame")

private:
    Int_t               fIPlot; ///< Select what to draw on main canvas by clicking on tab (0: map of inner/outer hole diameters, 1: profile plots, 2: std dev map of diameter, 3: density maps, 4: rim map)
    TGTransientFrame    * fMain;
    TGPictureButton     * load, * clear, * save, * print, * help;
    TGHorizontalFrame   * toolbar;
    TGStatusBar         * fStatusBar;
    TGTab               * fTab;
    TGCompositeFrame    * fCF[5];
    Double_t fXleft,fXright, fYbottom, fYtop;
    bool fLoadedSegmented;
    bool fLoadedUnsegmented;

protected:
    MOpt                * fOpt;
    TCanvas             * fCanv;
    TRootEmbeddedCanvas * fEcanvasAll[5];
    TPad                * fPad[8][5]; ///< 8 pads make space for labels, on five tabs

public:
    MOptFrame(Int_t location, const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
    virtual ~MOptFrame();

    void LoadFile();
    void DrawFoilNameLabel(Bool_t clear);
    void DoTab(Int_t);

    void AdjustPad(TPad * pad);
    void AdjustMapPad(TPad * pad);
    void CreateDividedPad2(Int_t itab);
    void CreateDividedPad4(Int_t itab);

    void DrawMaps(Int_t which_side);
    void DrawStdMaps(Int_t which_side);
    void DrawProfiles(Int_t which_side);
    void DrawDensityMaps(Int_t which_side);
    void DrawRimMaps(Int_t which_side);

    void EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();
    void Help();
    void Clear();
    void Print();
};

#endif // MOPTFRAME_H
