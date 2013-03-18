#!/bin/bash

#####################################################
# Unix jenkins script
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2013.03.16
#
# Description:
#
#   Continous integration build: run every time the repo changes.
#   Experimental!
#   Build part of jenkins CI
#
#       Download, build, and test CustusX
#       Publish unit tests
#
#
#####################################################

set -x
WORKSPACE=$1
CX_RELEASE_DIR=$WORKSPACE/working/CustusX3/build_Release

# ==========================================================
# Download, configure and build the project group.
# Python script will return success even if some parts failed.
./working/CustusX3/CustusX3/install/Shared/script/cxInstaller.py --full --all -t Release -j4 -s --isb_password=sintefsvn -u custusx --external_dir=$WORKSPACE/external --working_dir=$WORKSPACE/working --cmake_args="-DBUILD_DOCUMENTATION:BOOL=OFF"

# ==========================================================
# make the CustusX project in order to provoke a build failure.
cd $CX_RELEASE_DIR
make -j4
if [ $? == 0 ]
then
    echo "CustusX build success"
#    exit 0
else
    echo "CustusX build failure, terminating"
    exit 1
fi

# ==========================================================
# Run all tests and write them in xml format to ./CTestResults.xml
cd $CX_RELEASE_DIR
rm -rf Testing/201*
ctest -D ExperimentalTest --no-compress-output
cp Testing/`head -n 1 Testing/TAG`/Test.xml ./CTestResults.xml