/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

#ifndef CXTESTSESSIONSTORAGETESTFIXTURE_H
#define CXTESTSESSIONSTORAGETESTFIXTURE_H

#include "cxtest_org_custusx_core_patientmodel_export.h"

#include <QString>
#include <boost/shared_ptr.hpp>
#include "cxForwardDeclarations.h"
#include "cxImage.h"
#include "cxMesh.h"
#include <ctkPluginContext.h>

namespace cx
{
typedef boost::shared_ptr<class SessionStorageService> SessionStorageServicePtr;
typedef boost::shared_ptr<class PatientModelService> PatientModelServicePtr;
}

namespace cxtest
{
typedef boost::shared_ptr<class SessionStorageTestFixture> SessionStorageTestFixturePtr;

struct TestDataStructures
{
	cx::ImagePtr image1;
	cx::ImagePtr image2;
	cx::MeshPtr mesh1;
	TestDataStructures()
	{
		vtkImageDataPtr dummyImageData = cx::Image::createDummyImageData(2, 1);
		image1 = cx::ImagePtr(new cx::Image("imageUid1", dummyImageData, "imageName1"));
		image2 = cx::ImagePtr(new cx::Image("imageUid2", dummyImageData, "imageName2"));
		mesh1 = cx::Mesh::create("meshUid1","meshName1");
	}
};

class CXTEST_ORG_CUSTUSX_CORE_PATIENTMODEL_EXPORT SessionStorageTestFixture
{
public:
	SessionStorageTestFixture();

	~SessionStorageTestFixture();

	void createSessions();
	void loadSession1();
	void loadSession2();
	void reloadSession1();
	void reloadSession2();
	void saveSession();

	cx::SessionStorageServicePtr mSessionStorageService;
	cx::PatientModelServicePtr mPatientModelService;
	cx::VisServicesPtr mServices;
    ctkPluginContext* mContext;
private:
	bool mSessionsCreated;
	QString mSession1;
	QString mSession2;
};

} //cxtest
#endif // CXTESTSESSIONSTORAGETESTFIXTURE_H
