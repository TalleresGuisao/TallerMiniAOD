// -*- C++ -*-
//
// Package:    Demo/DemoAnalyzer_2
// Class:      DemoAnalyzer_2
//
/**\class DemoAnalyzer_2 DemoAnalyzer_2.cc Demo/DemoAnalyzer_2/plugins/DemoAnalyzer_2.cc

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


// ********** class declaration *****************

using reco::TrackCollection;

class DemoAnalyzer_2 : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit DemoAnalyzer_2(const edm::ParameterSet&);
  ~DemoAnalyzer_2() override;

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

  TH1D *demohisto_Mmumu;
};

//
// constructors and destructor
//
DemoAnalyzer_2::DemoAnalyzer_2(const edm::ParameterSet& iConfig)
  :
  //tracksToken_(consumes<TrackCollection>(iConfig.getUntrackedParameter<edm::InputTag>("tracks")))
  tracksToken_(consumes<edm::View<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("tracks"))),

  TrkPtmin_(iConfig.getParameter<double>("TrkPtmin")),
  TrkEtamax_(iConfig.getParameter<double>("TrkEtamax")),
  Mmin_(iConfig.getParameter<double>("Mmin")),
  Mmax_(iConfig.getParameter<double>("Mmax"))
{
  edm::Service<TFileService> fs;
  //demohisto_Mmumu = fs->make<TH1D>("Mmumu" , "Mmumu" , 80, 50, 130 );
  demohisto_Mmumu = fs->make<TH1D>("Mmumu" , "Mmumu" , 125, Mmin_, Mmax_ );
  
  //now do what ever initialization is needed
}


DemoAnalyzer_2::~DemoAnalyzer_2() { }

// **************** member functions *****************


// ------------ method called for each event  ------------
void DemoAnalyzer_2::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Track (pf) container
  edm::Handle< View<pat::PackedCandidate> > thePATTrackHandle;
  iEvent.getByToken(tracksToken_,thePATTrackHandle);

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

}

// ------------ method called once each job just before starting event loop  ------------
void DemoAnalyzer_2::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void DemoAnalyzer_2::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void DemoAnalyzer_2::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DemoAnalyzer_2);
