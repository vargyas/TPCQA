#include "mmainframe.cxx"

#include <TClass.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TVirtualPadEditor.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TGSlider.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGTextEdit.h>
#include <TGShutter.h>
#include <TGProgressBar.h>
#include <TGColorSelect.h>
#include <RQ_OBJECT.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>

/// Main window
/** This is the main window. */

void processFoil()
{
    new MMainFrame(gClient->GetRoot(), 700, 600);
}

//#ifdef STANDALONE
int main(int argc, char **argv)
{
   TApplication theApp("App", &argc, argv);

   if (gROOT->IsBatch()) 
   {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);  
      return 1;
   }
   processFoil();
   theApp.Run();
   return 0;
}
//#endif
