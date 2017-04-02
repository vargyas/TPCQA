#ifndef MOPTFRAME_H
#define MOPTFRAME_H

#include "mopt.cxx"


class MOptFrame
{
RQ_OBJECT("MOptFrame")

private:
	Int_t               fIPlot; ///< Select what to draw on main canvas by clicking on tab (0: map of inner/outer hole diameters, 1: profile plots, 2: std dev map of diameter, 3: density maps)
	TGTransientFrame    * fMain;
	TGPictureButton     * load, * clear, * save, * print, * help;
	TGHorizontalFrame   * toolbar;
	TGStatusBar         * fStatusBar;
	TGTab               * fTab;
	TGCompositeFrame    * fCF[4];
    Double_t fXleft,fXright, fYbottom, fYtop;
    bool fLoadedSegmented;
    bool fLoadedUnsegmented;

protected:
    MOpt                * fOpt;
	TCanvas             * fCanv;
	TRootEmbeddedCanvas * fEcanvasAll[4];
	TPad                * fPad[8][4]; ///< 8 pads make space for labels, on four tabs

public:
	MOptFrame(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
	virtual ~MOptFrame();

	void LoadFile();
    void DrawFoilNameLabel(Bool_t clear);
	void DoTab(Int_t);

    void AdjustPad(TPad * pad);
    void AdjustMapPad(TPad * pad);
    void CreateDividedPad();

    void DrawMaps(Int_t which_side);
	void DrawStdMaps(Int_t which_side);
    void DrawProfiles(Int_t which_side);
	void DrawDensityMaps(Int_t which_side);

	void EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
	void Save();
	void Help();
    void Clear();
	void Print();
};

#endif // MOPTFRAME_H
