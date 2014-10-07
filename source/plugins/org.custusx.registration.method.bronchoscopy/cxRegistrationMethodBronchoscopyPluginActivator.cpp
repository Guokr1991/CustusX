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

#include "cxRegistrationMethodBronchoscopyPluginActivator.h"

#include <QtPlugin>
#include <iostream>

#include "cxRegistrationMethodBronchoscopyService.h"
#include "cxRegisteredService.h"
#include "cxRegistrationServiceProxy.h"
#include "cxPatientModelServiceProxy.h"
#include "cxVisualizationServiceProxy.h"

namespace cx
{

RegistrationMethodBronchoscopyPluginActivator::RegistrationMethodBronchoscopyPluginActivator()
{
//	std::cout << "Created RegistrationMethodBronchoscopyPluginActivator" << std::endl;
}

RegistrationMethodBronchoscopyPluginActivator::~RegistrationMethodBronchoscopyPluginActivator()
{}

void RegistrationMethodBronchoscopyPluginActivator::start(ctkPluginContext* context)
{
	RegistrationServicePtr registrationService(new RegistrationServiceProxy(context));
	PatientModelServicePtr patientModelService(new PatientModelServiceProxy(context));
	VisualizationServicePtr visualizationService(new VisualizationServiceProxy(context));

	RegistrationMethodBronchoscopyImageToPatientService* image2patientService = new RegistrationMethodBronchoscopyImageToPatientService(registrationService, visualizationService, patientModelService);

	mRegistrationImageToPatient = RegisteredServicePtr(new RegisteredService(context, image2patientService, RegistrationMethodService_iid));
}

void RegistrationMethodBronchoscopyPluginActivator::stop(ctkPluginContext* context)
{
	mRegistrationImageToPatient.reset();
	Q_UNUSED(context);
}

} // namespace cx

Q_EXPORT_PLUGIN2(RegistrationMethodBronchoscopyPluginActivator_irrelevant_string, cx::RegistrationMethodBronchoscopyPluginActivator)


