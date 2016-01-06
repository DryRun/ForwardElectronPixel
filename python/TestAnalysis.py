import os
import sys
import datetime
import ROOT
from ROOT import *
gROOT.ProcessLine(".x $ROOTCOREDIR/scripts/load_packages.C")
gROOT.SetBatch(True)
sys.path.append("/global/homes/d/dryu/Luminosity/Code/RootUtils/python")
import Uniqify
import math
from math import *

def RunTestAnalysis():
	ROOT.xAOD.Init().ignore()
	sh = ROOT.SH.SampleHandler()
	ROOT.SH.ScanDir().sampleDepth(0).samplePattern("user.dryu.mc15_13TeV.361106.e3601_s2576_s2132_r7380.DAOD_IDTRKVALID.test4_EXT0").filePattern("*").scan(sh, "/eliza18/atlas/dryu/ForwardElectrons/Data/")
	sh.setMetaString ("nc_tree", "CollectionTree");
	sh.__getattribute__('print')()
	job = ROOT.EL.Job()
	job.sampleHandler(sh)
	
	# Output stream and ntuple service
	output_stream = ROOT.EL.OutputStream("TestAnalysis");
	job.outputAdd(output_stream);
	#output_ntuple_service = ROOT.EL.NTupleSvc("TrackCounts");
	#job.algsAdd(output_ntuple_service);

	# PixelClusterCounter algorithm
	alg = ROOT.TestAnalysis()
	alg.SetDataSource(ROOT.DataAccess.kSimulation)

	#job.options().setDouble(ROOT.EL.Job.optSkipEvents, 70000)
	#job.options().setDouble(ROOT.EL.Job.optMaxEvents, 100000) # Direct driver runs serially. If you comment this out, prepare for an hour wait!


	# Add algorithms to job and submit
	job.algsAdd(alg)
	driver = ROOT.EL.DirectDriver()
	submit_dir = "/eliza18/atlas/dryu/ForwardElectrons/Data/TestAnalysis/test_{}".format(datetime.datetime.now().strftime("%Y%m%d_%H%M%S"))
	driver.submit(job, submit_dir)

if __name__ == "__main__":
	#import argparse
	#parser = argparse.ArgumentParser(description = 'Submit track or pixel cluster counting jobs')
	#parser.add_argument('method', type=str, help='track or pcc')
	#parser.add_argument('id', type=str, help='Run or MC id')
	#parser.add_argument('submit_method', type=str, help='direct, batch, or grid')
	#args = parser.parse_args()
	RunTestAnalysis()
	print "All done."