#include "mlauncher.cxx"

/// Main window
/** This is the main window. */

void launcher()
{
    new MLauncher(gClient->GetRoot(), 900, 700);
}

//#ifdef STANDALONE
int main(int argc, char **argv)
{
   TApplication theApp("launcher", &argc, argv);

   if (gROOT->IsBatch()) 
   {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);  
      return 1;
   }
   launcher();

   theApp.Run();
   return 0;
}

//#endif
