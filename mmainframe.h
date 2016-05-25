/// Main Frame of the GUI application

#ifndef MMAINFRAME_H
#define MMAINFRAME_H

#include "mfoil.h"
#include "RQ_Object.h"
#include "TQObject.h"

#include "TApplication.h"
#include "TRootEmbeddedCanvas.h"
#include "TGButton.h"
#include "TGFileDialog.h"
#include "TGMsgBox.h"
#include "TGTab.h"
#include "TGStatusBar.h"
#include "TPolyMarker.h"
#include "TGLabel.h"



// main frame
class MMainFrame
{
RQ_OBJECT("MMainFrame")

private:
    TGMainFrame         * fMain;
	TGTransientFrame	* fSide;
	TRootEmbeddedCanvas * fEcanvasCh[24];
	TGStatusBar 		* fStatusBar;

    TCanvas 			* fCanv;
    MFoil 				* fFoil;
	TGTab				* fTab;

public:
    TRootEmbeddedCanvas * fEcanvasAll;

    MMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~MMainFrame();

    // slots
    void CloseWindow();
    void DoButton();
    void HandleMenu(Int_t id);
    void HandlePopup() { printf("menu popped up\n"); }
    void HandlePopdown() { printf("menu popped down\n"); }

    void Created(); //*SIGNAL*
    void Welcome();

    void DoExit();
    void LoadCurrentFile();
    void DrawFoilCurrents();
	void ProcessFoilCurrents();
	void DrawFoilCurrent(Int_t event, Int_t px, Int_t py, TObject * selected);
	Int_t ClickedOnPad(Int_t px, Int_t py);
	void EventInfo(Int_t event, Int_t px, Int_t py, TObject * selected);
};

#endif
