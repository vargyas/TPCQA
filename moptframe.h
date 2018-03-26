#ifndef MOPTFRAME_H
#define MOPTFRAME_H

#include "mopt.cxx"


class MOptFrame
{
RQ_OBJECT("MOptFrame")

private:
    Int_t               fIPlot; ///< Select what to draw on main canvas by clicking on tab (0: map of inner/outer hole diameters, 1: profile plots, 2: std dev map of diameter, 3: density maps, 4: rim map, 5: error maps)
    TGTransientFrame    * fMain;
    TGPictureButton     * loadH5, * loadROOT, * clear, * save, * print, * help;
    TGHorizontalFrame   * toolbar;
    TGStatusBar         * fStatusBar;
    TGTab               * fTab;
    TGCompositeFrame    * fCF[6];
    Double_t fXleft,fXright, fYbottom, fYtop;
    bool fLoadedSegmented;
    bool fLoadedUnsegmented;

protected:
    MOpt                * fOpt;
    TCanvas             * fCanv;
    TRootEmbeddedCanvas * fEcanvasAll[6];
    TPad                * fPad[10][6]; ///< maximum 10 pads make space for labels, on six tabs

public:
    MOptFrame(Int_t location, const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h);
    virtual ~MOptFrame();

    void LoadFileProtoScript(const TString infilename);
    void LoadFileProto(const TString indir, const char * filetypes[]);
    void LoadH5File();
    void LoadROOTFile();
    void DrawFoilNameLabel(Bool_t clear);
    void DoTab(Int_t);

    void AdjustPad(TPad * pad);
    void AdjustMapPad(TPad * pad);
    void AdjustErrorMapPad(TPad *pad);
    void CreateDividedPad2(Int_t itab);
    void CreateDividedPad4(Int_t itab);
    void CreateDividedPad6(Int_t itab);

    void DrawMaps(Int_t which_side);
    void DrawStdMaps(Int_t which_side);
    void DrawProfiles(Int_t which_side);
    void DrawDensityMaps(Int_t which_side);
    void DrawRimMaps(Int_t which_side);
    void DrawErrorMaps(Int_t which_side);

    void EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
    void Save();
    void Help();
    void Clear();
    void Print();
	void CloseWindow();

};

#endif // MOPTFRAME_H
