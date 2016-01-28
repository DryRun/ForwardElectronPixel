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
	histograms_->AddTH1D("medium_forward_electron_pt", "Forward electron p_{T}", "p_{T} [GeV]", 250, 0., 250.);
	histograms_->AddTH1D("loose_forward_electron_pt", "Forward electron p_{T}", "p_{T} [GeV]", 250, 0., 250.);

	histograms_->AddTH1D("forward_electron_eta", "Forward electron #eta", "#eta", 100, -5., 5.);
	histograms_->AddTH1D("medium_electron_eta", "Forward electron #eta", "#eta", 100, -5., 5.);
	histograms_->AddTH1D("tight_forward_electron_eta", "Forward electron #eta", "#eta", 100, -5., 5.);
	histograms_->AddTH1D("loose_forward_electron_eta", "Forward electron #eta", "#eta", 100, -5., 5.);

	histograms_->AddTH1D("forward_electron_phi", "Forward electron #phi", "#phi", 70, -3.5, 3.5);
	histograms_->AddTH1D("medium_electron_phi", "Forward electron #phi", "#phi", 70, -3.5, 3.5);
	histograms_->AddTH1D("tight_forward_electron_phi", "Forward electron #phi", "#phi", 70, -3.5, 3.5);
	histograms_->AddTH1D("loose_forward_electron_phi", "Forward electron #phi", "#phi", 70, -3.5, 3.5);

	histograms_->AddTH1D("pixel_cluster_size", "Pixel Cluster Size", "Pixel Cluster Size", 21, -0.5, 20.5);
	histograms_->AddTH1D("pixel_cluster_sizeZ", "Pixel Cluster SizeZ", "Pixel Cluster SizeZ", 21, -0.5, 20.5);

	histograms_->AddTH1D("el_pixelcluster_dphi", "#Delta#phi(cluster, e)", "#Delta#phi", 1120, -7., 7.);
	histograms_->AddTH1D("el_pixelcluster_dphi_fine", "#Delta#phi(cluster, e)", "#Delta#phi", 2000, -0.5, 0.5);
	histograms_->AddTH2D("el_pixelcluster_dphi_sizeZ", "#Delta#phi(cluster, e)", "#Delta#phi", 1120, -7., 7., "Pixel Cluster SizeZ", 21, -0.5, 20.5);
	histograms_->AddTH1D("el_pixelcluster_deta_dphi0.05", "#Delta#eta(cluster, e)", "#Delta#eta", 500, -5., 5.);
	histograms_->AddTH2D("el_pixelcluster_eta_sizeZ_dphi0.05", "#eta(cluster, e)", "#Delta#eta", 500, -5., 5., "Pixel Cluster SizeZ", 21, -0.5, 20.5);

	histograms_->AddTH1D("tp_loosecentral_loosefwd_eemass", "Tag (tight) / probe (loose) m_{ee}", "m_{ee} [GeV]", 400, 0., 200.);
	histograms_->AddTH1D("tp_loosecentral_loosefwd_pc_dphi", "Tag (tight) / probe (loose) #Delta#phi", "#Delta#phi", 1120, -7., 7.);
	histograms_->AddTH1D("tp_loosecentral_loosefwd_pc_dphi_fine", "Tag (tight) / probe (loose) #Delta#phi", "#Delta#phi", 2000, -0.5, 0.5);
	histograms_->AddTH2D("tp_loosecentral_loosefwd_pc_dphi_sizeZ", "#Delta#phi(cluster, e)", "#Delta#phi", 1120, -7., 7., "Pixel Cluster SizeZ", 21, -0.5, 20.5);
	histograms_->AddTH1D("tp_loosecentral_loosefwd_pc_deta_dphi0.05", "#Delta#eta(cluster, e)", "#Delta#eta", 500, -5., 5.);


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
	electron_LH_tight_ = new AsgElectronLikelihoodTool ("electron_LH_tight_");
	electron_LH_tight_->setProperty("primaryVertexContainer", "PrimaryVertices");
	std::string confDir = "ElectronPhotonSelectorTools/offline/mc15_20150712/";
	electron_LH_tight_->setProperty("ConfigFile",confDir+"ElectronLikelihoodTightOfflineConfig2015.conf");
	electron_LH_tight_->initialize();


	forward_electron_LH_tight_ = new  AsgForwardElectronIsEMSelector("forward_electron_LH_tight_");  // create the selector
    forward_electron_LH_tight_->setProperty("ConfigFile","ElectronPhotonSelectorTools/offline/mc15_20150812/ForwardElectronIsEMTightSelectorCutDefs.conf");
    forward_electron_LH_tight_->initialize();

	forward_electron_LH_medium_ = new  AsgForwardElectronIsEMSelector("forward_electron_LH_medium_");  // create the selector
    forward_electron_LH_medium_->setProperty("ConfigFile","ElectronPhotonSelectorTools/offline/mc15_20150812/ForwardElectronIsEMMediumSelectorCutDefs.conf");
    forward_electron_LH_medium_->initialize();

	forward_electron_LH_loose_ = new  AsgForwardElectronIsEMSelector("forward_electron_LH_loose_");  // create the selector
    forward_electron_LH_loose_->setProperty("ConfigFile","ElectronPhotonSelectorTools/offline/mc15_20150812/ForwardElectronIsEMLooseSelectorCutDefs.conf");
    forward_electron_LH_loose_->initialize();

	return EL::StatusCode::SUCCESS;
}



EL::StatusCode TestAnalysis::execute () {
	++processed_events_;
	if (processed_events_ < 10 || processed_events_ % 1000 == 0) {
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
	if (!this->TrackDataAccess::RetrieveTracks()) {
		Error("TestAnalysis::execute()", "TrackDataAccess::RetrieveTracks() failed.");
		exit(1);
	}

	if (processed_events_ < 10) {
		Info("TestAnalysis::execute()", "Done retrieving data.");
	}

	//xAOD::ElectronContainer forward_electrons_tight;
	//xAOD::ElectronContainer forward_electrons_medium;
	//xAOD::ElectronContainer forward_electrons_loose;
	for (xAOD::ElectronContainer::const_iterator it_fwdel = forward_electrons_->begin(); it_fwdel != forward_electrons_->end(); ++it_fwdel) {
		histograms_->GetTH1D("forward_electron_pt")->Fill((*it_fwdel)->pt() / 1000.);
		histograms_->GetTH1D("forward_electron_eta")->Fill((*it_fwdel)->eta());
		histograms_->GetTH1D("forward_electron_phi")->Fill((*it_fwdel)->phi());
		//if (forward_electron_LH_loose_->accept((*it_fwdel))) {
		if ((*it_fwdel)->auxdataConst<char>("Loose")) {
			//forward_electrons_loose.push_back(*it_fwdel);
			histograms_->GetTH1D("loose_forward_electron_pt")->Fill((*it_fwdel)->pt() / 1000.);
			histograms_->GetTH1D("loose_forward_electron_eta")->Fill((*it_fwdel)->eta());
			histograms_->GetTH1D("loose_forward_electron_phi")->Fill((*it_fwdel)->phi());
		}
		//if (forward_electron_LH_medium_->accept((*it_fwdel))) {
		//if ((*it_fwdel)->auxdataConst<char>("Medium")) {
		//	//forward_electrons_medium.push_back(*it_fwdel);
		//	histograms_->GetTH1D("medium_forward_electron_pt")->Fill((*it_fwdel)->pt() / 1000.);
		//	histograms_->GetTH1D("medium_forward_electron_eta")->Fill((*it_fwdel)->eta());
		//	histograms_->GetTH1D("medium_forward_electron_phi")->Fill((*it_fwdel)->phi());
		//}
		//if (forward_electron_LH_tight_->accept((*it_fwdel))) {
		if ((*it_fwdel)->auxdataConst<char>("Tight")) {
			//forward_electrons_tight.push_back(*it_fwdel);
			histograms_->GetTH1D("tight_forward_electron_pt")->Fill((*it_fwdel)->pt() / 1000.);
			histograms_->GetTH1D("tight_forward_electron_eta")->Fill((*it_fwdel)->eta());
			histograms_->GetTH1D("tight_forward_electron_phi")->Fill((*it_fwdel)->phi());
		}
	}

	if (processed_events_ < 10) {
		Info("TestAnalysis::execute()", "Done filling basic electron histograms");
	}


	for (xAOD::TrackMeasurementValidationContainer::const_iterator it_pc = pixel_clusters_->begin(); it_pc != pixel_clusters_->end(); ++it_pc) {
		histograms_->GetTH1D("pixel_cluster_size")->Fill((*it_pc)->auxdataConst<int>("size"));
		histograms_->GetTH1D("pixel_cluster_sizeZ")->Fill((*it_pc)->auxdataConst<int>("sizeZ"));
	}

	// Electron-cluster comparison
	// Electrons: forward, tight, pT>15GeV, |eta|<3.5
	for (xAOD::ElectronContainer::const_iterator it_fwdel = forward_electrons_->begin(); it_fwdel != forward_electrons_->end(); ++it_fwdel) {
	//for (auto& it_fwdel : forward_electrons_tight) {
		if (!(*it_fwdel)->auxdataConst<char>("Tight")) {
			continue;
		}
		if ((*it_fwdel)->pt() < 15000.) {
			continue;
		}
		if (fabs((*it_fwdel)->eta()) > 3.5) {
			continue;
		}

		for (xAOD::TrackMeasurementValidationContainer::const_iterator it_pc = pixel_clusters_->begin(); it_pc != pixel_clusters_->end(); ++it_pc) {
			double pc_x = (*it_pc)->globalX();
			double pc_y = (*it_pc)->globalY();
			double pc_z = (*it_pc)->globalZ();
			double pc_phi = TMath::ATan2(pc_y, pc_x);
			double pc_theta = TMath::ATan2(TMath::Sqrt(TMath::Power(pc_x, 2) + TMath::Power(pc_y, 2)), pc_z);
			double pc_eta = -1. * TMath::Log(TMath::Tan(pc_theta / 2.));

			// Loose eta cut
			double deta = (*it_fwdel)->eta() - pc_eta;
			double dphi = (*it_fwdel)->phi() - pc_phi;
			if (dphi < -1. * TMath::Pi()) {
				dphi = dphi + 2 * TMath::Pi();
			} else if (dphi > TMath::Pi()) {
				dphi = dphi - 2 * TMath::Pi();
			}

			// Require sizeZ>5
			if ((*it_pc)->auxdataConst<int>("sizeZ") <= 5) {
				// Fill dphi histograms with a loose eta cut
				if (TMath::Abs(deta) < 0.5) {
					histograms_->GetTH1D("el_pixelcluster_dphi")->Fill(dphi);
					histograms_->GetTH1D("el_pixelcluster_dphi_fine")->Fill(dphi);
					histograms_->GetTH2D("el_pixelcluster_dphi_sizeZ")->Fill(dphi, (*it_pc)->auxdataConst<int>("sizeZ"));
				}

				// Fill deta cut with a tight dphi cut
				if (TMath::Abs(dphi) < 0.05) {
					histograms_->GetTH1D("el_pixelcluster_deta_dphi0.05")->Fill(deta);
				}
			}
			if (TMath::Abs(dphi) < 0.05) {
				histograms_->GetTH2D("el_pixelcluster_eta_sizeZ_dphi0.05")->Fill((*it_fwdel)->eta(), (*it_pc)->auxdataConst<int>("sizeZ"));
			}
		}
	}

	/*** Tag and Probe ***/
	if (processed_events_ < 10) {
		Info("TestAnalysis::execute()", "Starting tag and probe. Getting primary vertex.");
	}

	xAOD::Vertex* vtx; // Need to identify primary vertex
	for (xAOD::VertexContainer::const_iterator vxIter = vertices_->begin(); vxIter != vertices_->end(); ++vxIter) {
		if ((*vxIter)->vertexType() == xAOD::VxType::PriVtx) {
			vtx = *vxIter;
		}
	}


	for (xAOD::ElectronContainer::const_iterator it_el = electrons_->begin(); it_el != electrons_->end(); ++it_el) {
		if (processed_events_ < 10) {
			//if (!(*it_el)->caloCluster()) {
			//	Error("TestAnalysis::execute()", "Electron caloCluster() is not valid.");
			//	exit(1);
			//}
			Info("TestAnalysis::execute()", "Doing central electron selection");
		}
		// 20 GeV
		if ((*it_el)->pt() < 20000.) {
			continue;
		}

		// |eta| < 2.47
		//if (fabs((*it_el)->caloCluster()->etaBE(2)) < 2.47 ) {
		if (fabs((*it_el)->eta()) < 2.47 ) {
			continue;
		}

		// Crack
		//if (fabs((*it_el)->caloCluster()->etaBE(2)) > 1.37 && fabs((*it_el)->caloCluster()->etaBE(2)) < 1.52) {
		if (fabs((*it_el)->eta()) > 1.37 && fabs((*it_el)->eta()) < 1.52) {
			continue;
		}

		// d0 significance < 5 and z0 sin theta < 0.5 mm
		const xAOD::TrackParticle *this_track = (*it_el)->trackParticle() ;
		//if (xAOD::TrackingHelpers::d0significance((*it_el)->trackParticle(), event_info_->beamPosSigmaX(), event_info_->beamPosSigmaY(), event_info_->beamPosSigmaXY()) > 5.0) {
		//	continue;
		//}
		//if (fabs(this_track->z0() + this_track->vz() - vtx->z()) * sin(this_track->theta()) > 0.5) {
		//	continue;
		//}

		// Tight LH
		//if (!(electron_LH_tight_->accept(*it_el))) {
		//	continue;
		//}
		// eg->selectionisEM("isEMLHLoose")
		if (!((*it_el)->selectionisEM("isEMLHLoose"))) {
			continue;
		}

		// We have a good candidate. Form Z candidates now.
		TLorentzVector tlv_tag;
		tlv_tag.SetPtEtaPhiM((*it_el)->pt() / 1000., (*it_el)->eta(), (*it_el)->phi(), Constants::ElectronMass(Constants::GeV));

		if (processed_events_ < 10) {
			Info("TestAnalysis::execute()", "Found tag, looping over probe candidates");
		}
		for (xAOD::ElectronContainer::const_iterator it_fwdel = forward_electrons_->begin(); it_fwdel != forward_electrons_->end(); ++it_fwdel) {

			// Require loose forward electrons
			//if (!forward_electron_LH_loose_->accept((*it_fwdel))) {
			//	continue;
			//}
			TLorentzVector tlv_probe;
			tlv_probe.SetPtEtaPhiM((*it_fwdel)->pt() / 1000., (*it_fwdel)->eta(), (*it_fwdel)->phi(), Constants::ElectronMass(Constants::GeV));
			double ee_mass = (tlv_tag + tlv_probe).M();
			histograms_->GetTH1D("tp_loosecentral_loosefwd_eemass")->Fill(ee_mass);
			if (TMath::Abs(ee_mass - Constants::ZMass(Constants::GeV)) < 10.) {
				// Good Zee pair. 
				for (xAOD::TrackMeasurementValidationContainer::const_iterator it_pc = pixel_clusters_->begin(); it_pc != pixel_clusters_->end(); ++it_pc) {
					// Require sizeZ>=3
					if ((*it_pc)->auxdataConst<int>("sizeZ") <= 2) {
						continue;
					}

					double pc_x = (*it_pc)->globalX();
					double pc_y = (*it_pc)->globalY();
					double pc_z = (*it_pc)->globalZ();
					double pc_phi = TMath::ATan2(pc_y, pc_x);
					double pc_theta = TMath::ATan2(TMath::Sqrt(TMath::Power(pc_x, 2) + TMath::Power(pc_y, 2)), pc_z);
					double pc_eta = -1. * TMath::Log(TMath::Tan(pc_theta / 2.));

					double deta = (*it_fwdel)->eta() - pc_eta;
					double dphi = (*it_fwdel)->phi() - pc_phi;
					if (dphi < -1. * TMath::Pi()) {
						dphi = dphi + 2 * TMath::Pi();
					} else if (dphi > TMath::Pi()) {
						dphi = dphi - 2 * TMath::Pi();
					}

					histograms_->GetTH1D("tp_loosecentral_loosefwd_pc_dphi")->Fill(dphi);
					histograms_->GetTH1D("tp_loosecentral_loosefwd_pc_dphi_fine")->Fill(dphi);
					histograms_->GetTH2D("tp_loosecentral_loosefwd_pc_dphi_sizeZ")->Fill(dphi, (*it_pc)->auxdataConst<int>("sizeZ"));
					histograms_->GetTH1D("tp_loosecentral_loosefwd_pc_deta_dphi0.05")->Fill(deta);
				}
			}
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
