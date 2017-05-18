#ifndef MOPT_H
#define MOPT_H

#include <algorithm>
#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<ctime>
#include<vector>


#include "TF1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"

#include "TMath.h"
#include "TVirtualFitter.h"
#include "TFile.h"
#include "TTree.h"
#include "TParameter.h"
#include "TString.h"

#include "TCanvas.h"
#include "TRootEmbeddedCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TLine.h"
#include "TPaveLabel.h"

#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TEnv.h"

#include "RQ_OBJECT.h"      // use this for linux
//#include "RQ_Object.h"            // use this for mac and win

#include "TQObject.h"
#include "TApplication.h"
#include "TGFileDialog.h"
#include "TGButton.h"
#include "TGMsgBox.h"
#include "TGTab.h"
#include "TGStatusBar.h"
#include "TPolyMarker.h"
#include "TGLabel.h"

#include <TClass.h>
#include <TVirtualX.h>
#include <TVirtualPadEditor.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGSlider.h>
#include <TGDoubleSlider.h>
#include <TGTextEdit.h>
#include <TGShutter.h>
#include <TGProgressBar.h>
#include <TGColorSelect.h>
#include <TColor.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>
#include <TFrame.h>

enum sides {kSegmented, kUnsegmented};
enum holetype {kInner, kOuter, kRim};

const char *filetypes_opt[] = { "ROOT files",    "*.root",
                                "Text files",    "*.[tT][xX][tT]",
                                "HDF5 files",    "*.h5",
                                "All files",     "*",
                                0,               0 };

class MOpt
{
RQ_OBJECT("MOpt")

private:
    Int_t fType;                ///< Type of the foil: 0=IROC, 1=OROC1, 2=OROC2, 3=OROC3, 4=else (not recognised)
    Int_t fLocation;
    TString fInFileName[2];     ///< Input file name
	TString fDataDir[2];
	TString fOutDir[2];
	TFile * fInFile[2];
    TString fName;              ///< Name of the foils as guessed from the directory name
    TTree * fTree[2][2];        ///< Input tree, S/U side, inner and outer
    TH2D * fhMapDiam[2][2];     ///< S/U side, inner and outer hole map histograms
    TH2D * fhMapStd[2][2];      ///< S/U side, inner and outer hole map histograms (calculated here)
    TH2D * fhMapN[2][2];
    TH2D * fhMapRim[2];         ///< S/U side rim map (calculated here)
    TH1D * fhProfDiam[2][3];
    TF1 * ffProfFit[2][3];      ///< Gaussian fit to profile diagrams
    Bool_t fIsLoaded[2];        ///<
    Double_t fConv;             ///< Conversion from pixel value to millimeter.
    Double_t fShift[2];         ///< Vector to shift the middle of foil to (0, 0)

public:
    MOpt(Int_t location);
    virtual ~MOpt();
    void CreateOutputContainers(Int_t which_side);
    void FillOutputContainers(Int_t which_side);
    void CalculateRim(Int_t which_side);
    void CalculateStd(Int_t which_side, Int_t which_hole);
    void GetOrigoShift(Int_t which_side);
    void GuessFoilName(const TString name); ///< Guess foil name from file/folder name
    void GuessFoilType(); ///< Guesses foil type from guessed name
	void LoadFile(const TString filename, const TString outputdir, Int_t which_side);
    void CloseFile(Int_t which_side);
    void DrawMaps(TPad * p, Int_t which_side, Int_t which_holetype, Int_t which_histo);
    void DrawFrame();
    void DrawProfiles(TPad * p, Int_t which_side);
    void Save();
    void SaveTxt();
    void SaveTxt1D();
    void SaveTxt2D();
    TString GetSaveName();
	TString GetROOTName(Int_t which_side);
    void DrawFitResult(Int_t which_side);
    void DrawOrigoShift(Int_t which_side);
    TString GetFitResult(Int_t which_side, Int_t which_hole);
    TString GetMeanWidth(Int_t which_side, Int_t which_hole);

    TString GetInFileName(Int_t i) {return fInFileName[i]; }
    TString GetFoilName() { return fName; }
};

#endif // MOPT_H
