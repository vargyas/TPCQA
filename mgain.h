#ifndef MGAIN_H
#define MGAIN_H

#include "TH2D.h"
#include "TH1D.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "RQ_OBJECT.h"

#include "TCanvas.h"
#include "TPad.h"

enum gaintype { kMeasurement, kReference, kGain };

class MGain
{
RQ_OBJECT("MGain")
  private:
	TString fInFileName[2];
	TFile * fInFile[2];
	TTree * fTree[2]; ///< Tree loaded from 0: measurement, 1: reference
	TH2D * fhMap[3]; ///< 0: Gain map of measurement, 1: reference, 2: actual gain
	TH1D * fProfMean[2];

  public:
	MGain();
	virtual ~MGain();
	void LoadFile(TString infilename, Int_t i);
	void Save();
	void DrawMap(TPad * p, Int_t which);
	void DrawProfile(TPad * p, Int_t which);

	void Clear(); ///< Since first load loads the mesurement, second the reference,
				  ///< it is convenient to reset the counter if one wants to reload each.
};

#endif // MGAIN_H
