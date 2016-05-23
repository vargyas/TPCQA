/// Container class for foil information

#ifndef MFOIL_H
#define MFOIL_H

#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<ctime>
#include<vector>

#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TFile.h"
#include "TTree.h"
#include "TEnv.h"
#include "TF1.h"
#include "TParameter.h"
#include "TString.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TRootEmbeddedCanvas.h"
#include "RQ_Object.h"
#include "TQObject.h"

class MFoil   
{
RQ_OBJECT("MFoil")

private: 
    Int_t fType;					///< Type of the foil: 0=IROC, 1=OROC
	Int_t fNumChannels;				///< Number of channels: 18 for IROC, 24 for OROC
    TString fName;						///< Name of the foil 
    TString fInFileName;				///< Name of the input file of the leakage current(s)

    Bool_t fFlagIsProcessed;			///< Status of processing of the foil, true if done, false if not
    Bool_t fFlagIsLoaded;				///< True if is loaded already, false if it is the first load of the datafile 
    Int_t fFlagQuality;						///< Quality of the foil, as determined by process. Possible values: 0=bad (red), 1=good (green), 2=problematic (orange)
	TString fQualityText;				///< Quality of the foil described by the operator
	
public:
    MFoil();                            ///< Default constructor
    MFoil( TString infilename);                           ///< Default constructor
	virtual ~MFoil();                   ///< Default destructor

    std::vector<Double_t> fSatVoltage;				///< Saturation voltage, as determined by Process()
	std::vector<Int_t> fNumSparks;					///< Number of sparks for each channel
    TList fhChannel;					///< Histograms of the leakage current
	TList fhChannelPos;					///< Histograms of the leakage current with absolute values (to work with TSpectrum, which only recongises positive peaks)
	std::vector<std::vector<Double_t>> fCurrents;

    void SetFileName(const TString infilename);

    void LoadFoilCurrents();			///< Loads foil information from file selected by mouse and displays it
    void ProcessFoilCurrents();         ///< Processes the measured foil current(s) and evaulates the foil
    void SaveFoil();                    ///< Saves obtained foil information and quality to either a pdf or a database
	Int_t GetNC();					///< Returns number of channels (fNumChannels)
	Int_t GetType();				///< Returns type fType
	Bool_t GetProcessedStatus();
};

#endif
