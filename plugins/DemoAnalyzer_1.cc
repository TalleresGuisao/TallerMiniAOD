// -*- C++ -*-
//
// Package:    Demo/DemoAnalyzer_1
// Class:      DemoAnalyzer_1
//
/**\class DemoAnalyzer_1 DemoAnalyzer_1.cc Demo/DemoAnalyzer_1/plugins/DemoAnalyzer_1.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Jhovanny Andres Mejia Guisao
//         Created:  Mon, 21 Sept 2026 18:46:47 GMT
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

class DemoAnalyzer_1 : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit DemoAnalyzer_1(const edm::ParameterSet&);
  ~DemoAnalyzer_1() override;

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

  TH1D *demohisto_ntracks;
  
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
DemoAnalyzer_1::DemoAnalyzer_1(const edm::ParameterSet& iConfig)
  :
  //tracksToken_(consumes<TrackCollection>(iConfig.getUntrackedParameter<edm::InputTag>("tracks")))
  tracksToken_(consumes<edm::View<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("tracks"))),

  TrkPtmin_(iConfig.getParameter<double>("TrkPtmin")),
  TrkEtamax_(iConfig.getParameter<double>("TrkEtamax"))
{
  edm::Service<TFileService> fs;
  demohisto_ntracks = fs->make<TH1D>("nTracks" , "nTracks" , 100 , 0 , 5000 );
  
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  setupDataToken_ = esConsumes<SetupData, SetupRecord>();
#endif
  //now do what ever initialization is needed
}

DemoAnalyzer_1::~DemoAnalyzer_1() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void DemoAnalyzer_1::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Track (pf) container
  edm::Handle< View<pat::PackedCandidate> > thePATTrackHandle;
  iEvent.getByToken(tracksToken_,thePATTrackHandle);

  // now loop on pf candidates
  
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
  demohisto_ntracks->Fill(nTrack);
  
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
void DemoAnalyzer_1::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void DemoAnalyzer_1::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void DemoAnalyzer_1::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
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
DEFINE_FWK_MODULE(DemoAnalyzer_1);
