# TallerMiniAOD
Set up your Environment
mkdir MYDEMOANALYZER
cd MYDEMOANALYZER
cmssw-el8	(singularity/apptainer link)
cmsrel CMSSW_13_0_13
cd CMSSW_13_0_13/src
cmsenv

Write a Framework Module
mkdir Demo
cd Demo
mkedanlzr DemoAnalyzer
cd DemoAnalyzer
scram b
