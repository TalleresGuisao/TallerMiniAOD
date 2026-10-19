// -*- C++ -*-
//
// Package:    Demo/DemoAnalyzer
// Class:      DemoAnalyzer
//
/**\class DemoAnalyzer DemoAnalyzer.cc Demo/DemoAnalyzer/plugins/DemoAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Jhovanny Andres Mejia Guisao
//         Created:  Wed, 09 Apr 2025 19:02:05 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/PatCandidates/interface/PackedCandidate.h" // Very important for MiniAOD tracks

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
//#include "TFile.h"
//#include "TTree.h"

#include "TLorentzVector.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

using reco::TrackCollection;

class DemoAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit DemoAnalyzer(const edm::ParameterSet&);
  ~DemoAnalyzer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  //edm::EDGetTokenT<TrackCollection> tracksToken_;  //used to select what tracks to read from configuration file
  edm::EDGetTokenT<edm::View<pat::PackedCandidate>> tracksToken_;

  double TrkPtmin_;
  double TrkEtamax_;
  double Mmin_;
  double Mmax_;

  TH1D *demohisto_ntracks;
  TH1D *demohisto_Mmumu;
  
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  edm::ESGetToken<SetupData, SetupRecord> setupToken_;
#endif
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
DemoAnalyzer::DemoAnalyzer(const edm::ParameterSet& iConfig)
  :
  //tracksToken_(consumes<TrackCollection>(iConfig.getUntrackedParameter<edm::InputTag>("tracks")))
  tracksToken_(consumes<edm::View<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("tracks"))),

  TrkPtmin_(iConfig.getParameter<double>("TrkPtmin")),
  TrkEtamax_(iConfig.getParameter<double>("TrkEtamax")),
  Mmin_(iConfig.getParameter<double>("Mmin")),
  Mmax_(iConfig.getParameter<double>("Mmax"))
{
  edm::Service<TFileService> fs;
  //demohisto_ntracks = fs->make<TH1D>("nTracks" , "nTracks" , 100 , 0 , 5000 );
  //demohisto_Mmumu = fs->make<TH1D>("Mmumu" , "Mmumu" , 80, 50, 130 );
  demohisto_Mmumu = fs->make<TH1D>("Mmumu" , "Mmumu" , 125, Mmin_, Mmax_ );
  
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  setupDataToken_ = esConsumes<SetupData, SetupRecord>();
#endif
  //now do what ever initialization is needed
}

DemoAnalyzer::~DemoAnalyzer() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void DemoAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Track (pf) container
  edm::Handle< View<pat::PackedCandidate> > thePATTrackHandle;
  iEvent.getByToken(tracksToken_,thePATTrackHandle);

  // now loop on pf candidates
  /*
  int nTrack = 0;
  for (unsigned int i = 0, n = thePATTrackHandle->size(); i < n; ++i) {
    const pat::PackedCandidate &pf = (*thePATTrackHandle)[i];
    //std::cout<< "Track pT: " << pf.pt() << std::endl;
    //if(pf.pt()>0.4 && fabs(pf.eta())<2.4){
    if(pf.pt()>TrkPtmin_ && fabs(pf.eta())<TrkEtamax_){
      nTrack++;
    }

  }

  //std::cout<<"nTrack = "<<nTrack<<std::endl;
  //demohisto_ntracks->Fill(nTrack);
  */

  double Mmumu = 0.0;
  for (unsigned int ii = 0, n = thePATTrackHandle->size(); ii < n; ++ii) {
    const pat::PackedCandidate &Track1 =  (*thePATTrackHandle)[ii];
    if(fabs(Track1.pdgId())!=13) continue;
    //if(fabs(Track1.pdgId())!=11) continue;
    if(Track1.pt()<TrkPtmin_) continue;
    if(fabs(Track1.eta())>TrkEtamax_) continue;
    
    for (unsigned int jj = ii+1, m = thePATTrackHandle->size(); jj<m ; ++jj ){
      const pat::PackedCandidate &Track2 =  (*thePATTrackHandle)[jj];
      if(fabs(Track2.pdgId())!=13) continue;
      //if(fabs(Track2.pdgId())!=11) continue;
      if(Track2.pt()<TrkPtmin_) continue;
      if(fabs(Track2.eta())>TrkEtamax_) continue;

      if (&Track1 == &Track2) continue;

      
      if (Track1.charge() * Track2.charge() > 0) continue;
      
      const Double_t mumass = 0.10566;
      TLorentzVector mu1_4V, mu2_4V, mumu_4v;
      mu1_4V.SetPtEtaPhiM(Track1.pt(),Track1.eta(),Track1.phi(),mumass);
      mu2_4V.SetPtEtaPhiM(Track2.pt(),Track2.eta(),Track2.phi(),mumass);

      mumu_4v = mu1_4V + mu2_4V ;
      const Double_t Mmumutmp = mumu_4v.M();
      //if(Mmumutmp>130.0 || Mmumutmp<50.0)continue;
      if(Mmumutmp>Mmax_ || Mmumutmp<Mmin_)continue;
      Mmumu = Mmumutmp;
    }
  }

  demohisto_Mmumu->Fill(Mmumu);
  
  /*
  for (const auto& track : iEvent.get(tracksToken_)) {  
    // do something with track parameters, e.g, plot the charge.
    // int charge = track.charge();
    std::cout<< "Track pT: " << track.pt() << std::endl;
  }
  */

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  // if the SetupData is always needed
  auto setup = iSetup.getData(setupToken_);
  // if need the ESHandle to check if the SetupData was there or not
  auto pSetup = iSetup.getHandle(setupToken_);
#endif
}

// ------------ method called once each job just before starting event loop  ------------
void DemoAnalyzer::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void DemoAnalyzer::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void DemoAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DemoAnalyzer);
