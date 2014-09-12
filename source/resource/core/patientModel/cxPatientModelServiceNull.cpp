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

#include "cxPatientModelServiceNull.h"
#include <map>
#include "cxData.h"
#include "cxReporter.h"
#include "cxLandmark.h"

namespace cx
{

PatientModelServiceNull::PatientModelServiceNull()
{
}
void PatientModelServiceNull::insertData(DataPtr data)
{
	printWarning();
}

void PatientModelServiceNull::updateRegistration_rMpr(const QDateTime& oldTime, const RegistrationTransform& newTransform)
{
	printWarning();
}

std::map<QString, DataPtr> PatientModelServiceNull::getData() const
{
	std::map<QString, DataPtr> retval;
	return retval;
}

DataPtr PatientModelServiceNull::getData(const QString& uid) const
{
	return DataPtr();
}

LandmarksPtr PatientModelServiceNull::getPatientLandmarks() const
{
	return boost::shared_ptr<Landmarks>();
}


std::map<QString, LandmarkProperty> PatientModelServiceNull::getLandmarkProperties() const
{
	return std::map<QString, LandmarkProperty>();
}

Transform3D PatientModelServiceNull::get_rMpr() const
{
	return Transform3D();
}

void PatientModelServiceNull::autoSave()
{
	printWarning();
}

bool PatientModelServiceNull::isNull()
{
	return true;
}

bool PatientModelServiceNull::getDebugMode() const
{
	return false;
}

void PatientModelServiceNull::setDebugMode(bool on)
{
	printWarning();
}

void PatientModelServiceNull::printWarning()
{
	reportWarning("Trying to use PatientModelServiceNull. Is PatientModelService (org.custusx.patiemtmodel) disabled?");
}

} // cx