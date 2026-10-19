import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.MessageLogger.cerr.FwkReport.reportEvery = 1000
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(5000) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(    
            
         ## https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2FDYTo*%2F*2022*%2FMINIAODSIM
        #'/store/mc/Run3Summer22EEMiniAODv3/DYToLL_M-50_TuneCP5_13p6TeV-pythia8/MINIAODSIM/Poisson60KeepRAW_124X_mcRun3_2022_realistic_postEE_v1-v2/2560000/0127e6b3-080a-46f7-83ef-57bc3af7a1af.root',
        'file:/afs/cern.ch/user/j/jmejiagu/public/TallerMiniAOD_UdeA/CMSSW_13_0_13/src/0127e6b3-080a-46f7-83ef-57bc3af7a1af.root',

 )
)

process.TFileService = cms.Service("TFileService",  fileName = cms.string('histodemo_2.root'))

process.demo = cms.EDAnalyzer('DemoAnalyzer_2',
                              tracks      = cms.InputTag('packedPFCandidates'),
                              TrkPtmin    = cms.double(0.4),
                              TrkEtamax   = cms.double(2.4),
                              Mmin    = cms.double(50.0),
                              Mmax    = cms.double(130.0),
                              )

process.p = cms.Path(process.demo)
