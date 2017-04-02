#ifndef MLAUNCHER_CXX
#define MLAUNCHER_CXX
#include "mlauncher.h"

MLauncher::MLauncher(const TGWindow *p, UInt_t w, UInt_t h)
{
	std::cout << "Your system arch. is: " << gSystem->GetBuildArch() << std::endl;
	//CpuInfo_t info;
	//gSystem->GetCpuInfo(&info);
	//std::cout << "\t your processor is: " << info << std::endl;


	fLaunch = new TGMainFrame(p, w, h);
	fToolbar = new TGHorizontalFrame(fLaunch,w,400);

	const char * welcomestring = "This is the unified launcher for TPC GEM QA.";

	fWelcome = new TGText(welcomestring);
	//fToolbar->AddFrame(fWelcome, new TGLayoutHints(kLHintsExpandX || kLHintsExpandY));

	fLeak = new TGTextButton(fToolbar, "LEAKAGE");
	//fLeak->SetPicture("scanner.png");
	fLeak->SetToolTipText ("Open leakage current measurement", 400);
	fLeak->Connect("Clicked()","MLauncher",this,"StartLeakage()");
	fLeak->Resize(800, 800);
	fToolbar->AddFrame(fLeak, new TGLayoutHints(kLHintsExpandX || kLHintsExpandY));

	fGain = new TGTextButton(fToolbar, "GAIN");
	fGain->SetToolTipText ("Open gain scan", 400);
	fGain->Connect("Clicked()","MLauncher",this,"StartGain()");
	fGain->Resize(80, 80);
	fToolbar->AddFrame(fGain, new TGLayoutHints(kLHintsExpandX|| kLHintsExpandY));

	fOpt = new TGTextButton(fToolbar, "OPTICAL");
	fOpt->SetToolTipText ("Open optical scan", 400);
	fOpt->Connect("Clicked()","MLauncher",this,"StartOptical()");
	fOpt->Resize(80, 80);
	fToolbar->AddFrame(fOpt, new TGLayoutHints(kLHintsExpandX|| kLHintsExpandY));

	//fLaunch->AddFrame()
	fLaunch->AddFrame(fToolbar,  new TGLayoutHints(kLHintsExpandX | kLHintsExpandY ));

	fLaunch->SetWindowName("TPC GEM QA Launcher");
	// Map all subwindows of main frame
	fLaunch->MapSubwindows();
	// Initialize the layout algorithm
	fLaunch->Resize(fLaunch->GetDefaultSize());
	// Map main frame
	fLaunch->MapWindow();
}

MLauncher::~MLauncher()
{
	//destructor
}

void MLauncher::StartLeakage()
{
	new MLeakFrame(gClient->GetRoot(), fLaunch, 900, 700);
}
void MLauncher::StartGain()
{
	//new MGainFrame(gClient->GetRoot(), fLaunch, 900, 700);
}
void MLauncher::StartOptical()
{
	new MOptFrame(gClient->GetRoot(), fLaunch, 900, 700);
}



#endif // MLAUNCHER_CXX
