// -*- C++ -*-
//
// Package:    Demo/Analyzer_JpsiTrk
// Class:      Analyzer_JpsiTrk
//
/**\class Analyzer_JpsiTrk Analyzer_JpsiTrk.cc Demo/DemoAnalyzer/plugins/Analyzer_JpsiTrk.cc

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

#include "DataFormats/PatCandidates/interface/PackedCandidate.h" // MiniAOD tracks
#include "DataFormats/PatCandidates/interface/Muon.h"            

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"

//For kinematic fit:
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

using reco::TrackCollection;

class Analyzer_JpsiTrk : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit Analyzer_JpsiTrk(const edm::ParameterSet&);
  ~Analyzer_JpsiTrk() override;

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<edm::View<pat::PackedCandidate>> tracksToken_;
  edm::EDGetTokenT<edm::View<pat::Muon>> dimuon_Label;
  edm::EDGetTokenT<reco::VertexCollection> primaryVertices_Label;
  edm::EDGetTokenT<reco::BeamSpot> BSLabel_;

  double TrkPtmin_;
  double TrkEtamax_;
  double Mmin_;
  double Mmax_;

  // ****************** Define our Tree ******************
  TTree*      tree_;
  UInt_t    run;
  ULong64_t event;
  UInt_t    lumiblock;

  Double_t    Bmass, Bpt, Beta, Bphi;
  
  Double_t    Mmumu, ptmumu, etamumu, phimumu;
  Double_t    mu1pt, mu1eta, mu1phi;
  Double_t    mu2pt, mu2eta, mu2phi;
  
  Double_t    Trk1pt, Trk1eta, Trk1phi;
  
  bool        mu1soft, mu2soft, mu1tight, mu2tight;  
  bool        mu1medium, mu2medium, mu1loose, mu2loose;

  Double_t    priVtx_Prob;
  UInt_t      nVtx;
  
};

//
// constructors and destructor
//
Analyzer_JpsiTrk::Analyzer_JpsiTrk(const edm::ParameterSet& iConfig)
  :
  tracksToken_(consumes<edm::View<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("tracks"))),
  dimuon_Label(consumes<edm::View<pat::Muon>>(iConfig.getParameter<edm::InputTag>("dimuons"))),
  primaryVertices_Label(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertices"))),
  BSLabel_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("bslabel"))),

  TrkPtmin_(iConfig.getParameter<double>("TrkPtmin")),
  TrkEtamax_(iConfig.getParameter<double>("TrkEtamax")),
  Mmin_(iConfig.getParameter<double>("Mmin")),
  Mmax_(iConfig.getParameter<double>("Mmax")),

  // ****************** initialize variables for Tree ******************
  tree_(0),
  run(0), event(0),
  lumiblock(0),

  Bmass(0), Bpt(0), Beta(0), Bphi(0),
  
  Mmumu(0), ptmumu(0), etamumu(0), phimumu(0),
  mu1pt(0), mu1eta(0), mu1phi(0),
  mu2pt(0), mu2eta(0), mu2phi(0),

  Trk1pt(0), Trk1eta(0), Trk1phi(0),
  
  mu1soft(0), mu2soft(0), mu1tight(0), mu2tight(0), 
  mu1medium(0), mu2medium(0), mu1loose(0), mu2loose(0),

  priVtx_Prob(0), nVtx(0)
{
  //now do what ever initialization is needed
}

Analyzer_JpsiTrk::~Analyzer_JpsiTrk() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void Analyzer_JpsiTrk::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  //*********************************
  // Get event "id" information 
  //*********************************
  lumiblock = iEvent.id().luminosityBlock();
  run = iEvent.id().run();
  event = iEvent.id().event();

  //*********************************
  // Get event content information
  //*********************************  

  // Track (pf) container
  edm::Handle< View<pat::PackedCandidate> > thePATTrackHandle;
  iEvent.getByToken(tracksToken_,thePATTrackHandle);

  // Muon container
  edm::Handle< View<pat::Muon> > thePATMuonHandle;
  iEvent.getByToken(dimuon_Label,thePATMuonHandle);

  //*********************************
  //Get the primary vertex 
  //*********************************
  reco::Vertex bestVtx;
  edm::Handle<std::vector<reco::Vertex> > recVtxs;
  iEvent.getByToken(primaryVertices_Label, recVtxs);

  bestVtx = *(recVtxs->begin());
  priVtx_Prob = ChiSquaredProbability((double)(bestVtx.chi2()),(double)(bestVtx.ndof())); 
  nVtx = recVtxs->size();

  for(View<pat::Muon>::const_iterator iMuon1 = thePATMuonHandle->begin(); iMuon1 != thePATMuonHandle->end(); ++iMuon1){
    if( iMuon1->track().isNull() ) continue;
    if(iMuon1->track()->pt()<TrkPtmin_) continue;
    if(fabs(iMuon1->track()->eta())>TrkEtamax_) continue;
    
    for(View<pat::Muon>::const_iterator iMuon2 = iMuon1+1; iMuon2 != thePATMuonHandle->end(); ++iMuon2) {
      if(iMuon1==iMuon2) continue;
      
      if( iMuon2->track().isNull() ) continue;   
      if(iMuon2->track()->pt()<TrkPtmin_) continue;
      if(fabs(iMuon2->track()->eta())>TrkEtamax_) continue;

      //opposite charge 
      if( (iMuon1->charge())*(iMuon2->charge()) == 1) continue;

      //Quality cuts?? Muon Id?
      if(!(iMuon1->track()->quality(reco::TrackBase::highPurity))) continue;
      if(!(iMuon2->track()->quality(reco::TrackBase::highPurity))) continue;
      
      const Double_t mumass = 0.10566;
      TLorentzVector mu1_4V, mu2_4V, mumu_4v;
      mu1_4V.SetPtEtaPhiM(iMuon1->track()->pt(),iMuon1->track()->eta(),iMuon1->track()->phi(),mumass);
      mu2_4V.SetPtEtaPhiM(iMuon2->track()->pt(),iMuon2->track()->eta(),iMuon2->track()->phi(),mumass);

      mumu_4v = mu1_4V + mu2_4V ;
      Double_t Mmumutmp = mumu_4v.M();
      if(Mmumutmp>Mmax_ || Mmumutmp<Mmin_)continue;

      //Now that we have a J/psi candidate, we look for Trk candidates
      for(View<pat::PackedCandidate>::const_iterator iTrack1 = thePATTrackHandle->begin(); iTrack1 != thePATTrackHandle->end(); ++iTrack1 ) {
	if(iTrack1->charge()==0) continue;
	if(fabs(iTrack1->pdgId())!=211) continue;
	if(iTrack1->pt()<0.95) continue;
	if(!(iTrack1->trackHighPurity())) continue;
	if(iTrack1->numberOfPixelHits()<1)continue;
	if(iTrack1->numberOfHits()<5)continue;

	const Double_t Trkmass = 0.493677;
	TLorentzVector Trk1_4V, mumutrk_4v;
	Trk1_4V.SetPtEtaPhiM(iTrack1->pt(),iTrack1->eta(),iTrack1->phi(),Trkmass);

	mumutrk_4v = mu1_4V + mu2_4V + Trk1_4V ;
	Double_t Bmasstmp = mumutrk_4v.M();
	if(Bmasstmp>5.8 || Bmasstmp<4.8)continue;
	

	//fill the tree
	Bmass   = Bmasstmp;
	Bpt     = mumutrk_4v.Pt();
	Beta    = mumutrk_4v.Eta();
	Bphi    = mumutrk_4v.Phi();
	
	Mmumu   = Mmumutmp;
	ptmumu  = mumu_4v.Pt();
	etamumu = mumu_4v.Eta();
	phimumu = mumu_4v.Phi();

	mu1pt   = iMuon1->track()->pt();
	mu1eta  = iMuon1->track()->eta();
	mu1phi  = iMuon1->track()->phi();

	mu2pt   = iMuon2->track()->pt();
	mu2eta  = iMuon2->track()->eta();
	mu2phi  = iMuon2->track()->phi();

	Trk1pt  = iTrack1->pt();
	Trk1eta = iTrack1->eta();
	Trk1phi = iTrack1->phi();
	
	mu1soft = iMuon1->isSoftMuon(bestVtx) ;
	mu2soft = iMuon2->isSoftMuon(bestVtx) ;
	mu1tight = iMuon1->isTightMuon(bestVtx) ;
	mu2tight = iMuon2->isTightMuon(bestVtx) ;
	mu1medium = muon::isMediumMuon(*iMuon1);
	mu2medium = muon::isMediumMuon(*iMuon2);
	mu1loose = muon::isLooseMuon(*iMuon1);
	mu2loose = muon::isLooseMuon(*iMuon2);
		   
	tree_->Fill();
      }
    }
  }

  // ****************** clean variables for Tree ******************
  run = 0; event = 0;  lumiblock = 0;

  Bmass = 0; Bpt = 0; Beta = 0; Bphi = 0;
  
  Mmumu = 0; ptmumu = 0; etamumu = 0; phimumu = 0;
  mu1pt = 0; mu1eta = 0; mu1phi = 0;
  mu2pt = 0; mu2eta = 0; mu2phi = 0;

  Trk1pt = 0; Trk1eta = 0; Trk1phi = 0;
  
  mu1soft = 0; mu2soft = 0; mu1tight = 0; mu2tight = 0;
  mu1medium = 0; mu2medium = 0; mu1loose = 0; mu2loose = 0;

  priVtx_Prob = 0; nVtx = 0;

}

// ------------ method called once each job just before starting event loop  ------------
void Analyzer_JpsiTrk::beginJob() {
  std::cout << "Beginning analyzer job" << std::endl;

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("ntuple","X->JpsiTrk ntuple");

  tree_->Branch("run",        &run,       "run/I");
  tree_->Branch("event",        &event,     "event/L");
  tree_->Branch("lumiblock",&lumiblock,"lumiblock/I");

  //tree_->Branch("", &);
  tree_->Branch("Bmass", &Bmass);
  tree_->Branch("Bpt", &Bpt);
  tree_->Branch("Beta", &Beta);
  tree_->Branch("Bphi", &Bphi);

  tree_->Branch("Mmumu", &Mmumu);
  tree_->Branch("ptmumu", &ptmumu);
  tree_->Branch("etamumu", &etamumu);
  tree_->Branch("phimumu", &phimumu);

  tree_->Branch("mu1pt", &mu1pt);
  tree_->Branch("mu1eta", &mu1eta);
  tree_->Branch("mu1phi", &mu1phi);
  tree_->Branch("mu2pt", &mu2pt);
  tree_->Branch("mu2eta", &mu2eta);
  tree_->Branch("mu2phi", &mu2phi);

  tree_->Branch("Trk1pt", &Trk1pt);
  tree_->Branch("Trk1eta", &Trk1eta);
  tree_->Branch("Trk1phi", &Trk1phi);

  tree_->Branch("mu1soft",&mu1soft);
  tree_->Branch("mu2soft",&mu2soft);
  tree_->Branch("mu1tight",&mu1tight);
  tree_->Branch("mu2tight",&mu2tight);
  tree_->Branch("mu1medium",&mu1medium);
  tree_->Branch("mu2medium",&mu2medium);
  tree_->Branch("mu1loose",&mu1loose);
  tree_->Branch("mu2loose",&mu2loose);

  tree_->Branch("priVtx_Prob",&priVtx_Prob, "priVtx_Prob/D");
  tree_->Branch("nVtx",       &nVtx);
  
}

// ------------ method called once each job just after ending the event loop  ------------
void Analyzer_JpsiTrk::endJob() {
  tree_->GetDirectory()->cd();
  tree_->Write();
}

//define this as a plug-in
DEFINE_FWK_MODULE(Analyzer_JpsiTrk);
