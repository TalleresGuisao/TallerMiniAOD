# TallerMiniAOD

## Set up your Environment

```bash
mkdir MYDEMOANALYZER
cd MYDEMOANALYZER
```

## Set up CMSSW

```bash
cmsrel CMSSW_13_0_13
cd CMSSW_13_0_13/src
cmsenv
```

## Write a Framework Module

```bash
mkdir Demo
cd Demo
mkedanlzr DemoAnalyzer
cd DemoAnalyzer
scram b
```
