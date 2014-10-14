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

#include "cxRegistrationPluginActivator.h"

#include <QtPlugin>
#include <iostream>

#include "cxRegistrationImplService.h"
#include "cxRegisteredService.h"

namespace cx
{

RegistrationPluginActivator::RegistrationPluginActivator()
{
}

RegistrationPluginActivator::~RegistrationPluginActivator()
{
}

void RegistrationPluginActivator::start(ctkPluginContext* context)
{
	mRegistration = RegisteredService::create<RegistrationImplService>(context, RegistrationService_iid);


	//TODO: publish additional registration widgets
//	RegistrationHistoryWidgetService *i2i = new RegistrationHistoryWidgetService(services);
//	WirePhantomWidgetService *i2i = new WirePhantomWidgetService(services);
//	mRegistrationHistory = RegisteredServicePtr(new RegisteredService(context, i2i, GUIExtenderService_iid));
//	mWirePhantomWidget = RegisteredServicePtr(new RegisteredService(context, i2i, GUIExtenderService_iid));

	//TODO: Create services for these
//	mRegistrationHistory = GUIExtenderService::CategorizedWidget(new RegistrationHistoryWidget(NULL), "Browsing");
//	mWirePhantomWidget = GUIExtenderService::CategorizedWidget(new WirePhantomWidget(mRegistrationService, mVisualizationService, mPatientModelService, mAquisitionData, NULL), "Algorithms");


}

void RegistrationPluginActivator::stop(ctkPluginContext* context)
{
	mRegistration.reset();
//	mRegistrationHistory.reset();
//	mWirePhantomWidget.reset();
	Q_UNUSED(context);
}

} // namespace cx



