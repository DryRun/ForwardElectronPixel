#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <ForwardElectronPixel/TestAnalysis.h>

// this is needed to distribute the algorithm to the workers
ClassImp(TestAnalysis)



TestAnalysis::TestAnalysis () {
	histograms_ = 0;
	data_source_ = kCollisionData;
}

TestAnalysis::~TestAnalysis() {
}



EL::StatusCode TestAnalysis::setupJob (EL::Job& job) {
	job.useXAOD();
	if (!(xAOD::Init()).isSuccess()) {
		Error("IDLumiCounter::setupJob()", "xAOD::Init() failed.");
		exit(1);
	}
	xAOD::TReturnCode::enableFailure();
	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::histInitialize () {
	IgnoreBranch("egammaClusters");
	IgnoreBranch("ForwardElectronClusters");

	histograms_ = new Root::HistogramManager();
	histograms_->AddPrefix("h_");
	histograms_->SetELOutput(wk());
	histograms_->AddTH1D("forward_electron_pt", "Forward electron p_{T}", "p_{T} [GeV]", 250, 0., 250.);
	histograms_->AddTH1D("tight_forward_electron_pt", "Forward electron p_{T}", "p_{T} [GeV]", 250, 0., 250.);
	histograms_->AddTH1D("loose_forward_electron_pt", "Forward electron p_{T}", "p_{T} [GeV]", 250, 0., 250.);

	histograms_->AddTH1D("forward_electron_eta", "Forward electron #eta", "#eta", 100, -5., 5.);
	histograms_->AddTH1D("tight_forward_electron_eta", "Forward electron #eta", "#eta", 100, -5., 5.);
	histograms_->AddTH1D("loose_forward_electron_eta", "Forward electron #eta", "#eta", 100, -5., 5.);

	histograms_->AddTH1D("forward_electron_phi", "Forward electron #phi", "#phi", 70, -3.5, 3.5);
	histograms_->AddTH1D("tight_forward_electron_phi", "Forward electron #phi", "#phi", 70, -3.5, 3.5);
	histograms_->AddTH1D("loose_forward_electron_phi", "Forward electron #phi", "#phi", 70, -3.5, 3.5);

	histograms_->AddTH1D("pixel_cluster_size", "Pixel Cluster Size", "Pixel Cluster Size", 21, -0.5, 20.5);
	histograms_->AddTH1D("pixel_cluster_sizeZ", "Pixel Cluster SizeZ", "Pixel Cluster SizeZ", 21, -0.5, 20.5);

	histograms_->AddTH1D("el_pixelcluster_dphi", "#Delta#phi(cluster, e)", "#Delta#phi", 1120, -7., 7.);
	histograms_->AddTH2D("el_pixelcluster_dphi_sizeZ", "#Delta#phi(cluster, e)", "#Delta#phi", 1120, -7., 7., "Pixel Cluster SizeZ", 21, -0.5, 20.5);

	processed_events_ = 0;

	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::fileExecute () {
	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::changeInput (bool firstFile) {
	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::initialize () {
	event_ = wk()->xaodEvent();
	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::execute () {
	++processed_events_;
	if (processed_events_ < 10) {
		Info("TestAnalysis::execute()", "Processing event %i", processed_events_);
	}
	event_ = wk()->xaodEvent();
	if (!this->EventInfoDataAccess::RetrieveEventInfo()) {
		Error("TestAnalysis::execute()", "EventInfoDataAccess::RetrieveEventInfo() failed.");
		exit(1);
	}
	if (!this->PixelClusterDataAccess::RetrievePixelClusters()) {
		Error("TestAnalysis::execute()", "PixelClusterDataAccess::RetrievePixelClusters() failed.");
		exit(1);
	}
	if (!this->ElectronDataAccess::RetrieveElectrons()) {
		Error("TestAnalysis::execute()", "ElectronDataAccess::RetrieveElectrons() failed.");
		exit(1);
	}
	for (auto it_fwdel = forward_electrons_->begin(); it_fwdel != forward_electrons_->end(); ++it_fwdel) {
		histograms_->GetTH1D("forward_electron_pt")->Fill((*it_fwdel)->pt() / 1000.);
		histograms_->GetTH1D("forward_electron_eta")->Fill((*it_fwdel)->eta());
		histograms_->GetTH1D("forward_electron_phi")->Fill((*it_fwdel)->phi());
		if ((*it_fwdel)->auxdataConst<char>("Loose")) {
			histograms_->GetTH1D("loose_forward_electron_pt")->Fill((*it_fwdel)->pt() / 1000.);
			histograms_->GetTH1D("loose_forward_electron_eta")->Fill((*it_fwdel)->eta());
			histograms_->GetTH1D("loose_forward_electron_phi")->Fill((*it_fwdel)->phi());
		}
		if ((*it_fwdel)->auxdataConst<char>("Tight")) {
			histograms_->GetTH1D("tight_forward_electron_pt")->Fill((*it_fwdel)->pt() / 1000.);
			histograms_->GetTH1D("tight_forward_electron_eta")->Fill((*it_fwdel)->eta());
			histograms_->GetTH1D("tight_forward_electron_phi")->Fill((*it_fwdel)->phi());
		}
	}

	for (auto it_pc = pixel_clusters_->begin(); it_pc != pixel_clusters_->end(); ++it_pc) {
		histograms_->GetTH1D("pixel_cluster_size")->Fill((*it_pc)->auxdataConst<int>("size"));
		histograms_->GetTH1D("pixel_cluster_sizeZ")->Fill((*it_pc)->auxdataConst<int>("sizeZ"));
	}

	// Electron-cluster comparison
	for (auto it_fwdel = forward_electrons_->begin(); it_fwdel != forward_electrons_->end(); ++it_fwdel) {
		// Tight forward electrons only
		if (!((*it_fwdel)->auxdataConst<char>("Tight"))) {
			continue;
		}
		for (auto it_pc = pixel_clusters_->begin(); it_pc != pixel_clusters_->end(); ++it_pc) {
			// Require sizeZ>5
			if ((*it_pc)->auxdataConst<int>("sizeZ") <= 5) {
				continue;
			}

			double pc_x = (*it_pc)->globalX();
			double pc_y = (*it_pc)->globalY();
			double pc_z = (*it_pc)->globalZ();
			double pc_phi = TMath::ATan2(pc_y, pc_x);
			double pc_theta = TMath::ATan2(TMath::Sqrt(TMath::Power(pc_x, 2) + TMath::Power(pc_y, 2)), pc_z);
			double pc_eta = -1. * TMath::Log(TMath::Tan(pc_theta / 2.));

			// Loose eta cut
			if (TMath::Abs((*it_fwdel)->eta() - pc_eta) > 0.5) {
				continue;
			}

			double dphi = (*it_fwdel)->phi() - pc_phi;
			if (dphi < -1. * TMath::Pi()) {
				dphi = dphi + 2 * TMath::Pi();
			} else if (dphi > TMath::Pi()) {
				dphi = dphi - 2 * TMath::Pi();
			}

			histograms_->GetTH1D("el_pixelcluster_dphi")->Fill(dphi);
			histograms_->GetTH2D("el_pixelcluster_dphi_sizeZ")->Fill(dphi, (*it_pc)->auxdataConst<int>("sizeZ"));
		}
	}

	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::postExecute () {
	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::finalize () {
	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::histFinalize () {
	if (histograms_) {
		delete histograms_;
		histograms_ = 0;
	}

	return EL::StatusCode::SUCCESS;
}
