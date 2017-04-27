#ifndef MGAINFRAME_H
#define MGAINFRAME_H

#include "mgain.cxx"

enum sides {kSegmented, kUnsegmented};

class MGainFrame
{
RQ_OBJECT("MGainFrame")

private:
    Int_t               fIPlot; ///< Select what to draw on main canvas by clicking on tab (0: map of gain, 1: profile plots)
    TGTransientFrame    * fMain;
    TGPictureButton     * load, * save, * help;
    TGHorizontalFrame   * toolbar;
    TGStatusBar         * fStatusBar;
    TGTab               * fTab;
    TGCompositeFrame    * fCF[3];
    Double_t fXmin, fYmin;//, fXRight, fYLeft, fYRight;//

protected:
    MGain               * fGain; // 1: unsegmented (bottom panel), 0: segmented side (top panel)
    TCanvas             * fCanv;
    TRootEmbeddedCanvas * fEcanvasAll[3];
    TPad                * fPad[5][3]; ///< Four pads make space for labels, on three tabs

public:
    MGainFrame(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
    virtual ~MGainFrame();

    void LoadFile();
    void DoTab(Int_t);

    void AdjustPad(TPad * pad);
    void AdjustMapPad(TPad * pad);
    void CreateDividedPad();

    void DrawMaps();
    void DrawProfiles();

    void EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();
    void Help();
};

#endif // MGAINFRAME_H
