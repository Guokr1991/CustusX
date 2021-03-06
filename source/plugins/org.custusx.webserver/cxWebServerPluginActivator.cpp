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

#include "cxWebServerPluginActivator.h"

#include <QtPlugin>
#include <iostream>

#include "cxRegisteredService.h"
#include <QtConcurrent>
#include <QNetworkAccessManager>
#include <QHostAddress>
#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>
#include "cxScreenVideoProvider.h"

#include "cxPatientModelService.h"
#include "cxHttpRequestHandler.h"
#include "cxRemoteAPI.h"
#include "cxLogger.h"

#include <QPixmap>

namespace cx
{

NetworkPluginActivator::NetworkPluginActivator()
{
    std::cout << "Created NetworkPluginActivator" << std::endl;
}

NetworkPluginActivator::~NetworkPluginActivator()
{}

void NetworkPluginActivator::start(ctkPluginContext* context)
{
    int port = 8085;
    CX_LOG_INFO() << QString("Starting REST server on port %1...").arg(port);
	server = new QHttpServer;
	mAPI.reset(new RemoteAPI(VisServices::create(context)));
	mRequestHandler.reset(new HttpRequestHandler(mAPI));
	connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
			mRequestHandler.get(), SLOT(handle_request(QHttpRequest*, QHttpResponse*)));

	// let's go
    server->listen(port);

	//mRegistration = RegisteredService::create<NetworkServiceImpl>(context, NetworkService_iid);
}

void NetworkPluginActivator::stop(ctkPluginContext* context)
{
	delete server;
	server = NULL;
//	mRegistration.reset();
	Q_UNUSED(context);
}


} // namespace cx



