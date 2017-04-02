#ifndef MLAUNCHER_H
#define MLAUNCHER_H

#include "mleakframe.cxx"
#include "moptframe.cxx"
#include "mgainframe.cxx"

class MLauncher
{
RQ_OBJECT("MLauncher")

  private:
	TGMainFrame * fLaunch;
	TGText * fWelcome;
    TGHorizontalFrame * fToolbar;
	TGTextButton * fOpt, *fLeak, *fGain;
  
  public:
	MLauncher(const TGWindow *p, UInt_t w, UInt_t h);
	virtual ~MLauncher();
    void StartLeakage(); // *SIGNAL*
    void StartOptical(); // *SIGNAL*
    void StartGain(); // *SIGNAL*
};

#endif // MLAUNCHER_H
