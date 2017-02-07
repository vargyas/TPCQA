/// Container class for foil information

#ifndef MFOIL_H
#define MFOIL_H

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

#include "TROOT.h"
#include "TStyle.h"
#include "TEnv.h"

#include "RQ_Object.h"
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


const char *filetypes[] = { "All files",     "*",
                            "Text files",    "*.[tT][xX][tT]",
                            "ROOT files",    "*.root",
                            0,               0 };

//----------------------------------------------------------------------------------
Double_t String2Sec(const std::string timestring)
{
    const char * _ctime = timestring.c_str();
    std::istringstream _ss(_ctime);
    std::string _token;
    double _times_temp;
    double _times[3];
    int _itimes = 0;

    while (std::getline(_ss, _token, ':'))
    {
        std::istringstream(_token) >> _times_temp;
        _times[_itimes++] = _times_temp;
    }
    return _times[0] * 60 * 60 + _times[1] * 60 + _times[2];
}
//----------------------------------------------------------------------------------
Double_t AverageHist(TH1D * h)
{
    Int_t n = h->GetNbinsX();
    Double_t val = 0;
    for(Int_t ib = 0; ib < n; ++ib)
    {
        val += h->GetBinContent(ib+1);
    }
    return val / Double_t(n);
}
//----------------------------------------------------------------------------------
// find x-limits (first and last non-zero bins)
Double_t * GetXlimits(TH1D * h)
{
    Int_t count  = 0;
    Double_t val = 0;
    Double_t eps = 1E-10;
    Double_t * limits = new Double_t[2];

    for(Int_t ib = 1; ib <= h->GetNbinsX(); ++ib)
    {
        val = h->GetBinContent(ib);
        //std::cout << val << endl;

        if(val < -eps && val > eps) // instead of != 0
        {
            ++count;
            if(count == 1) limits[0] = val;
            else limits[1] = val;
        }
    }
    std::cout << "limits are: " << limits[0] << "\t" << limits[1] << std::endl;
    return limits;
}
//----------------------------------------------------------------------------------
// own median finder
Double_t GetMedian(TH1D * h)
{
    Int_t n = h->GetNbinsX();
    std::vector<double> y;
    y.reserve(n);
    for(Int_t ib = 1; ib <= n; ++ib)
        y.push_back(h->GetBinContent(ib));

    std::sort(y.begin(), y.end());
    Double_t median_y = y.at(n/2);

    //Int_t median_index;
    //h->GetBinWithContent( median_y, median_index );
    //Double_t median_x = h->GetBinCenter(median_index);
    return median_y;
}



class MFoil   
{
RQ_OBJECT("MFoil")

private: 
    TTree * fInTree;                    ///< Tree directly read from inputfile, contains all information from the measurement
    Int_t fType;                        ///< Type of the foil: 0=IROC, 1=OROC1, 2=OROC2, 3=OROC3, 4=else
    Int_t fNumChannels;                 ///< Number of channels: 18 for IROC, 24 for OROC
    TString fName;                      ///< Name of the foil 
    TString fInFileName;                ///< Name of the input file of the leakage current(s)

    Bool_t fFlagIsProcessed;            ///< Status of processing of the foil, true if done, false if not
    Bool_t fFlagIsLoaded;               ///< True if is loaded already, false if it is the first load of the datafile 
    std::vector<Int_t> fFlagQuality;    ///< Quality of the foil, as determined by process for each channel. Possible values: 0=bad (red), 1=good (green), 2=problematic (orange)
    TString fQualityText;               ///< Quality of the foil described by the operator
    TH1D * fhLimitTime;                 ///< Histogram holding the leakage current limit (0.5nA in most cases)
    TH1D * fhLimitStd;                  ///< Dummy histogram showing the 1sigma limit on the standard deviation plot
    Double_t fMeasurementStart;         ///< Start of measurement, after ramping is finished, determined by DetectMeasurementStart()
    Double_t fMeasurementEnd;           ///< End of measurement, determined by DetectMeasurementStop()
    Double_t fLimit;                    ///< Acceptance leakage current limit in nA
    TList fhCurrentTime;                ///< List of time-dependent leakage current graphs
    TList fhCurrentStd;                 ///< List of leakage current distribution histograms
    std::vector<Double_t> fSatCurrent;  ///< Saturation current in nA

    void SetFileName(const TString infilename);
    
    void SaveFoil();                    ///< Saves obtained foil information and quality to either a pdf or a database

public:
    MFoil();                            ///< Default constructor
    virtual ~MFoil();                   ///< Default destructor

    void LoadFoilCurrents(const TString infilename);    ///< Loads foil information from file selected by mouse and displays it
    void ProcessFoilCurrents();         ///< Processes the measured foil current(s) and evaulates the foil
    
    Int_t GetNC();                      ///< Returns number of channels (fNumChannels)
    Int_t GetType();                    ///< Returns type of foil (fType)
    Bool_t GetProcessedStatus() const;
    Bool_t GetLoadedStatus() const;
    TString GetInfoSatCurrent(Int_t id) const;
    TString GetInFileName() const;
    TString GetName() const;

    void CreateHLimitTime(); ///<
    void CreateHLimitStd(Int_t ich, Double_t ymax); ///<
    void DrawHLimitTime(Int_t ich);
    void DrawHLimitStd(Int_t ich);
    void DrawCurrentTime(Int_t id, TCanvas * c);
    void DrawCurrentStd(Int_t ich, TCanvas * c);
    void DrawCurrentCorr(Int_t ich, TCanvas * c);

    void DrawSatCurrent(Int_t ich);
    void DrawMeasurementRange(Int_t ich);
    void DrawSparks(Int_t ich, TString opt);
    void DrawCurrentTimeAll(Int_t ich, TCanvas * c);

    Double_t DetectMeasurementStart();
    Double_t DetectMeasurementStop();
    void DetectSparks(Double_t xmax);
    Double_t EstimateSatCurrent(Int_t id);
    Double_t GetSaturationCurrent(Int_t id);
    
    Int_t GetProcessedColor(Int_t ich) const ;
    Double_t GetLastSparkPosition(Int_t ich);

    void SetPadMargins(TCanvas * c);
    void SetAxisStyle(TH1D * h);
    
    //ClassDef(MFoil, 1);
};

#endif
