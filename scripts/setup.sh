#!/bin/bash

# Make directories
#export FWD_EL_DIR=/eliza18/atlas/wmccorma/ForwardElectrons
export FWD_EL_DIR=/eliza18/atlas/dryu/ForwardElectronDemo
mkdir -pv $FWD_EL_DIR
mkdir -pv $FWD_EL_DIR/Derivation
mkdir -pv $FWD_EL_DIR/Analysis

# Setup derivation area
cp /eliza18/atlas/dryu/ForwardElectrons/Derivation/new_shell.sh $FWD_EL_DIR/Derivation
cd $FWD_EL_DIR/Derivation
source new_shell.sh

# Authenticate. This might require the SSH configurations in ~dryu/.ssh/config
kinit -f wmccorma@CERN.CH

# Start checking out packages
pkgco.py InDetPrepRawDataToxAOD
pkgco.py DerivationFrameworkCore
pkgco.py DerivationFrameworkEGamma
pkgco.py DerivationFrameworkInDet
pkgco.py PrimaryDPDMaker
pkgco.py ElectronPhotonSelectorTools

# Copy modified file from David's area
cp /eliza18/atlas/dryu/ForwardElectrons/Derivation/InnerDetector/InDetEventCnv/InDetPrepRawDataToxAOD/share/InDetDxAOD.py $FWD_EL_DIR/Derivation/InnerDetector/InDetEventCnv/InDetPrepRawDataToxAOD/share

# Compile etc. This makes it so that the modified InDetDxAOD.py file is used, rather than the unedited one.
cd $TestArea
setupWorkArea.py
cd $TestArea/WorkArea/cmt
cmt bro cmt config
cmt bro gmake

# Run a test job
cd $TestArea/WorkArea/run 
cp /eliza18/atlas/dryu/ForwardElectrons/Derivation/workdir/*IDTRKVALID*sh . # Copy submission scripts
sed -i "s_/eliza18/atlas/dryu/ForwardElectrons/Derivation_$FWD_EL_DIR/Derivation_" ./*IDTRKVALID*.sh # Change the hard-coded paths

# Submit derivation job to the batch system. 
# '-l eliza18io=1' is required by the administrators to specify that the job uses eliza18. 
# '-l h_vmem=6G' specifies a minimum of 6G memory
qsub -l eliza18io=1 -l h_vmem=6G batch_test_ESD_to_DAOD_IDTRKVALID.sh 
