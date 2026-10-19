// -*- C++ -*-
//
// Package:    Demo/Analyzer_JpsiTrk_two
// Class:      Analyzer_JpsiTrk_two
//
/**\class Analyzer_JpsiTrk_two Analyzer_JpsiTrk_two.cc Demo/DemoAnalyzer/plugins/Analyzer_JpsiTrk_two.cc

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
#include "DataFormats/PatCandidates/interface/GenericParticle.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TVector3.h"

//For kinematic fit:
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"

#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/TransientTrackKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/VertexPrimitives/interface/BasicSingleVertexState.h"
#include "RecoVertex/VertexPrimitives/interface/VertexState.h"
#include "RecoVertex/AdaptiveVertexFit/interface/AdaptiveVertexFitter.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"

#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackFromFTSFactory.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"
#include "MagneticField/Engine/interface/MagneticField.h"            

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

using reco::TrackCollection;

class Analyzer_JpsiTrk_two : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit Analyzer_JpsiTrk_two(const edm::ParameterSet&);
  ~Analyzer_JpsiTrk_two() override;
  bool IsTheSame(const pat::PackedCandidate& tk, const pat::Muon& mu);
  //bool IsTheSame(const pat::GenericParticle& tk, const pat::Muon& mu);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  edm::ESGetToken<TransientTrackBuilder, TransientTrackRecord> ttrkToken_;
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

  Double_t    B_Prob, mumu_Prob;
  Double_t    ppdlPV;
  Double_t    ppdlErrPV;
  Double_t    cosAlpha;
  Double_t    priVtx_Prob;
  UInt_t      nVtx;

  int         tri_JpsiTk, tri_DMu4_3_LM;   
};

//
// constructors and destructor
//
Analyzer_JpsiTrk_two::Analyzer_JpsiTrk_two(const edm::ParameterSet& iConfig)
  :
  ttrkToken_(esConsumes<TransientTrackBuilder, TransientTrackRecord>(edm::ESInputTag("", "TransientTrackBuilder"))),
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
  
  B_Prob(0), mumu_Prob(0),
  ppdlPV(0), ppdlErrPV(0), cosAlpha(0), priVtx_Prob(0), nVtx(0),

  tri_JpsiTk(0),tri_DMu4_3_LM(0)
{
  //now do what ever initialization is needed
}

Analyzer_JpsiTrk_two::~Analyzer_JpsiTrk_two() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void Analyzer_JpsiTrk_two::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  using std::vector;

  //*********************************
  // Get event "id" information 
  //*********************************
  lumiblock = iEvent.id().luminosityBlock();
  run = iEvent.id().run();
  event = iEvent.id().event();

  //*********************************
  // Get event content information
  //*********************************
  // Kinematic fit
  edm::ESHandle<TransientTrackBuilder> theB = iSetup.getHandle(ttrkToken_); 
  
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
      

      //Let's check the vertex and mass
      reco::TransientTrack muon1TT((*theB).build(iMuon1->track()));
      reco::TransientTrack muon2TT((*theB).build(iMuon2->track()));

      //The mass of a muon and the insignificant mass sigma 
      ParticleMass muon_mass = 0.10565837; //pdg mass
      float muon_sigma = muon_mass*1.e-6;

      //Creating a KinematicParticleFactory
      KinematicParticleFactoryFromTransientTrack pFactory;

      //initial chi2 and ndf before kinematic fits.
      float chi = 0.;
      float ndf = 0.;
      vector<RefCountedKinematicParticle> muonParticles;
      muonParticles.push_back(pFactory.particle(muon1TT,muon_mass,chi,ndf,muon_sigma));
      muonParticles.push_back(pFactory.particle(muon2TT,muon_mass,chi,ndf,muon_sigma));

      KinematicParticleVertexFitter fitter;   
      RefCountedKinematicTree psiVertexFitTree;
      psiVertexFitTree = fitter.fit(muonParticles);
      if (!psiVertexFitTree->isValid()) {
	//std::cout << "caught an exception in the psi vertex fit" << std::endl;
	continue; 
      }

      psiVertexFitTree->movePointerToTheTop();
      RefCountedKinematicParticle psi_vFit_noMC = psiVertexFitTree->currentParticle();
      RefCountedKinematicVertex psi_vFit_vertex_noMC = psiVertexFitTree->currentDecayVertex();

      if(psi_vFit_noMC->currentState().mass()<Mmin_ || psi_vFit_noMC->currentState().mass()>Mmax_) continue;

      double J_Prob_tmp   = TMath::Prob(psi_vFit_vertex_noMC->chiSquared(),(int)psi_vFit_vertex_noMC->degreesOfFreedom());
      if(J_Prob_tmp<0.001)continue;

      //Now that we have a J/psi candidate, we look for Trk candidates
      for(View<pat::PackedCandidate>::const_iterator iTrack1 = thePATTrackHandle->begin(); iTrack1 != thePATTrackHandle->end(); ++iTrack1 ) {
	if(iTrack1->charge()==0) continue;
	if(fabs(iTrack1->pdgId())!=211) continue;
	//if(iTrack1->pt()<0.95) continue;
	//if(iTrack1->pt()<1.2) continue;// strong?
	if(!(iTrack1->trackHighPurity())) continue;
	if(iTrack1->numberOfPixelHits()<1)continue;
	if(iTrack1->numberOfHits()<5)continue;

	//Now let's checks if our muons do not use the same tracks as we are using now
	if ( IsTheSame(*iTrack1,*iMuon1) || IsTheSame(*iTrack1,*iMuon2) ) continue;

	reco::TransientTrack kaonTT((*theB).build(iTrack1->pseudoTrack()));
	ParticleMass pion_mass = 0.493677;
	float pion_sigma = pion_mass*1.e-6;

	float chi = 0.;
	float ndf = 0.;

	//Now we are ready to combine!
	// JPsi mass constraint is applied in the final B fit,
	vector<RefCountedKinematicParticle> vFitMCParticles;
	vFitMCParticles.push_back(pFactory.particle(muon1TT,muon_mass,chi,ndf,muon_sigma));
	vFitMCParticles.push_back(pFactory.particle(muon2TT,muon_mass,chi,ndf,muon_sigma));
	vFitMCParticles.push_back(pFactory.particle(kaonTT,pion_mass,chi,ndf,pion_sigma));

	RefCountedKinematicTree vertexFitTree;
	ParticleMass psi_mass = 3.096916;
	//float psi_sigma = psi_mass*1.e-6; 
	MultiTrackKinematicConstraint *  j_psi_c = new  TwoTrackMassKinematicConstraint(psi_mass);
	KinematicConstrainedVertexFitter kcvFitter;
	vertexFitTree = kcvFitter.fit(vFitMCParticles, j_psi_c);
	if (!vertexFitTree->isValid()) {
	  //std::cout << "caught an exception in the B vertex fit with MC" << std::endl;
	  continue;
	}
	
	vertexFitTree->movePointerToTheTop();
	RefCountedKinematicParticle bCandMC = vertexFitTree->currentParticle();
	RefCountedKinematicVertex bDecayVertexMC = vertexFitTree->currentDecayVertex();
	if (!bDecayVertexMC->vertexIsValid()){
	  //std::cout << "B MC fit vertex is not valid" << endl;
	  continue;
	}

	if( (bCandMC->currentState().mass() < 5.0) || (bCandMC->currentState().mass() > 5.6) ) { continue; }

	double B_Prob_tmp  = TMath::Prob(bDecayVertexMC->chiSquared(),(int)bDecayVertexMC->degreesOfFreedom());
	if(B_Prob_tmp<0.001) continue;
	
	// ********************* lifetime using PV ****************
	reco::Vertex::Point BvtxPos(bDecayVertexMC->position().x(), bDecayVertexMC->position().y(), bDecayVertexMC->position().z());
	reco::Vertex::Error Berr = bDecayVertexMC->error().matrix(); // Error: is SMatrixSym3D, an simetric 3x3matrix .
	double myBchi2 = bDecayVertexMC->chiSquared();
	double myBndof = bDecayVertexMC->degreesOfFreedom();
	reco::Vertex BtheVtx(BvtxPos, Berr, myBchi2, myBndof, 0);
	//SMatrixSym3D BtotalCov = bestVtx.covariance() + BtheVtx.covariance();
		   
	TVector3 vtx;
	TVector3 pvtx;
	VertexDistanceXY vdistXY;

	vtx.SetXYZ( bDecayVertexMC->position().x(), bDecayVertexMC->position().y(), 0);
	TVector3 pperp(bCandMC->currentState().globalMomentum().x(), bCandMC->currentState().globalMomentum().y(), 0);
	AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0);

	pvtx.SetXYZ(bestVtx.x(), bestVtx.y(), 0);
	TVector3 vdiff = vtx - pvtx;

	cosAlpha = vdiff.Dot(pperp) / (vdiff.Perp() * pperp.Perp());
	//Measurement1D distXY = vdistXY.distance(reco::Vertex(bDecayVertexMC), bestVtx);
	Measurement1D distXY = vdistXY.distance(BtheVtx, bestVtx);
	ppdlPV = distXY.value() * cosAlpha * bCandMC->currentState().mass() / pperp.Perp();
	GlobalError v1e = bDecayVertexMC->error();
	GlobalError v2e = bestVtx.error();
	AlgebraicSymMatrix33 vXYe = v1e.matrix() + v2e.matrix();
	ppdlErrPV = sqrt(ROOT::Math::Similarity(vpperp, vXYe)) * bCandMC->currentState().mass() / (pperp.Perp2());

	//fill variables
	Bmass   = bCandMC->currentState().mass();
	Bpt     = bCandMC->currentState().globalMomentum().perp();
	Beta    = bCandMC->currentState().globalMomentum().eta();
	Bphi    = bCandMC->currentState().globalMomentum().phi();
	
	Mmumu   = psi_vFit_noMC->currentState().mass();
	ptmumu  = psi_vFit_noMC->currentState().globalMomentum().perp();
	etamumu = psi_vFit_noMC->currentState().globalMomentum().eta();
	phimumu = psi_vFit_noMC->currentState().globalMomentum().phi();

	mu1pt   = iMuon1->track()->pt();
	mu1eta  = iMuon1->track()->eta();
	mu1phi  = iMuon1->track()->phi();

	mu2pt   = iMuon2->track()->pt();
	mu2eta  = iMuon2->track()->eta();
	mu2phi  = iMuon2->track()->phi();

	Trk1pt  = iTrack1->pt();
	Trk1eta = iTrack1->eta();
	Trk1phi = iTrack1->phi();

	B_Prob     = B_Prob_tmp;
	mumu_Prob   = J_Prob_tmp;
	
	mu1soft = iMuon1->isSoftMuon(bestVtx) ;
	mu2soft = iMuon2->isSoftMuon(bestVtx) ;
	mu1tight = iMuon1->isTightMuon(bestVtx) ;
	mu2tight = iMuon2->isTightMuon(bestVtx) ;
	mu1medium = muon::isMediumMuon(*iMuon1);
	mu2medium = muon::isMediumMuon(*iMuon2);
	mu1loose = muon::isLooseMuon(*iMuon1);
	mu2loose = muon::isLooseMuon(*iMuon2);

	// ********************* muon-trigger-machint ****************
	const pat::Muon* muon1 = &(*iMuon1);
	const pat::Muon* muon2 = &(*iMuon2);

	int tri_JpsiTk_tmp = 0,  tri_DMu4_3_LM_tmp = 0;
	if(muon1->triggerObjectMatchByPath("HLT_DoubleMu4_MuMuTrk_Displaced_v*")!=nullptr && muon2->triggerObjectMatchByPath("HLT_DoubleMu4_MuMuTrk_Displaced_v*")!=nullptr) tri_JpsiTk_tmp = 1;
	if(muon1->triggerObjectMatchByPath("HLT_DoubleMu4_3_LowMass_v*")!=nullptr && muon2->triggerObjectMatchByPath("HLT_DoubleMu4_3_LowMass_v*")!=nullptr) tri_DMu4_3_LM_tmp = 1;

	tri_JpsiTk =  tri_JpsiTk_tmp ;
	tri_DMu4_3_LM =  tri_DMu4_3_LM_tmp ;

	//fill the tree	   
	tree_->Fill();
	muonParticles.clear();
	vFitMCParticles.clear();
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
  
  B_Prob = 0; mumu_Prob = 0;
  ppdlPV = 0;  ppdlErrPV = 0; cosAlpha = 0; priVtx_Prob = 0; nVtx = 0;

  tri_JpsiTk = 0;  tri_DMu4_3_LM = 0; 

}

bool Analyzer_JpsiTrk_two::IsTheSame(const pat::PackedCandidate& tk, const pat::Muon& mu){
  double DeltaEta = fabs(mu.eta()-tk.eta());
  double DeltaP   = fabs(mu.p()-tk.p());
  if (DeltaEta < 0.02 && DeltaP < 0.02) return true;
  return false;
}

// ------------ method called once each job just before starting event loop  ------------
void Analyzer_JpsiTrk_two::beginJob() {
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

  tree_->Branch("B_Prob",    &B_Prob);
  tree_->Branch("mumu_Prob",  &mumu_Prob);
  tree_->Branch("ppdlPV",&ppdlPV, "ppdlPV/D");
  tree_->Branch("ppdlErrPV",&ppdlErrPV, "ppdlErrPV/D");
  tree_->Branch("cosAlpha",&cosAlpha, "cosAlpha/D");
  tree_->Branch("priVtx_Prob",&priVtx_Prob, "priVtx_Prob/D");
  tree_->Branch("nVtx",       &nVtx);

  tree_->Branch("tri_JpsiTk",&tri_JpsiTk);
  tree_->Branch("tri_DMu4_3_LM",&tri_DMu4_3_LM);
  
}

// ------------ method called once each job just after ending the event loop  ------------
void Analyzer_JpsiTrk_two::endJob() {
  tree_->GetDirectory()->cd();
  tree_->Write();
}

//define this as a plug-in
DEFINE_FWK_MODULE(Analyzer_JpsiTrk_two);
