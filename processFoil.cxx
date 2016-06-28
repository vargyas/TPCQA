//#include "procFoilLinkDef.h"
#include "mmainframe.cxx"

/// Main window
/** This is the main window. */

void processFoil()
{
	new MMainFrame(gClient->GetRoot(), 900, 700);
}

//#ifdef STANDALONE
int main(int argc, char **argv)
{
   TApplication theApp("processFoil", &argc, argv);

   if (gROOT->IsBatch()) 
   {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);  
      return 1;
   }
   processFoil();
   //new MMainFrame(gClient->GetRoot(), 900, 700);

   theApp.Run();
   return 0;
}

//#endif
