import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.Services_cff')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, '124X_dataRun3_Prompt_v4' '') # for 2022 CDEF
process.GlobalTag = GlobalTag(process.GlobalTag, '124X_dataRun3_Prompt_v10' '') # for 2022 G
#process.GlobalTag = GlobalTag(process.GlobalTag, '130X_dataRun3_HLT_v2')      #for CMSSW >= 13_0_3

## Message Logger and Event range
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(5000) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        ## Using Xrootd Service (AAA) for Remote Data Access 
        ## https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookXrootdService
            
        #dataset=/ParkingDoubleMuonLowMass*/Run2022*-PromptReco*/MINIAOD 
        #'/store/data/Run2022G/ParkingDoubleMuonLowMass1/MINIAOD/PromptReco-v1/000/362/433/00000/fc62fe8f-6936-4e74-874c-b7b6d86fa7a4.root',
        #'/store/data/Run2022G/ParkingDoubleMuonLowMass2/MINIAOD/PromptReco-v1/000/362/435/00000/a007706c-89be-4ce3-9dba-a902c8bf035c.root',
        #'/store/mc/Run3Summer23BPixMiniAODv4/ButoJpsiK_Jpsito2Mu_TuneCP5_13p6TeV_pythia8-evtgen/MINIAODSIM/130X_mcRun3_2023_realistic_postBPix_v2-v3/2810000/0884233e-16e0-40a1-8f28-41f7b929ecad.root'
        'file:/afs/cern.ch/user/j/jmejiagu/public/TallerMiniAOD/CMSSW_13_0_13/src/0884233e-16e0-40a1-8f28-41f7b929ecad.root',

 )
)

process.TFileService = cms.Service("TFileService",  fileName = cms.string('Rootuple_JpsiTrk.root'))

process.demo = cms.EDAnalyzer('Analyzer_JpsiTrk',
                              tracks      = cms.InputTag('packedPFCandidates'),
                              dimuons     = cms.InputTag("slimmedMuons"),
                              primaryVertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
                              bslabel = cms.InputTag("offlineBeamSpot"),
                              TrkPtmin    = cms.double(3.0),
                              TrkEtamax   = cms.double(2.4),
                              Mmin    = cms.double(0.2),
                              Mmax    = cms.double(4.0),
                              )

process.p = cms.Path(process.demo)
