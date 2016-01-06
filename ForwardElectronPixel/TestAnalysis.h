#ifndef ForwardElectronPixel_TestAnalysis_H
#define ForwardElectronPixel_TestAnalysis_H

#include <string>

#include <TROOT.h>
#include <TSystem.h>

#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include <EventLoop/Algorithm.h>
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>

#include "xAODRootAccess/Init.h"
#include "RootUtils/HistogramManager.h"
#include "AnalysisTools/DataAccess.h"
#include "AnalysisTools/EventInfoDataAccess.h"
#include "AnalysisTools/PixelClusterDataAccess.h"
#include "AnalysisTools/ElectronDataAccess.h"



class TestAnalysis : public EL::Algorithm, public EventInfoDataAccess, public PixelClusterDataAccess, public ElectronDataAccess
{
public:
	TestAnalysis();
	~TestAnalysis();

	// these are the functions inherited from Algorithm
	virtual EL::StatusCode setupJob (EL::Job& job);
	virtual EL::StatusCode fileExecute ();
	virtual EL::StatusCode histInitialize ();
	virtual EL::StatusCode changeInput (bool firstFile);
	virtual EL::StatusCode initialize ();
	virtual EL::StatusCode execute ();
	virtual EL::StatusCode postExecute ();
	virtual EL::StatusCode finalize ();
	virtual EL::StatusCode histFinalize ();

	// this is needed to distribute the algorithm to the workers
	ClassDef(TestAnalysis, 1);

private:

public:

private:
	int processed_events_; //!
	Root::HistogramManager *histograms_; //!

};

#endif
