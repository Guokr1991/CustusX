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

#ifndef CXIMAGESERVER_H_
#define CXIMAGESERVER_H_

#include "cxGrabberExport.h"

#include <QTcpServer>
#include <QTimer>
#include <QPointer>
#include "cxStreamer.h"

namespace cx
{

/**
 * \brief ImageServer
 *
 * \ingroup cx_resource_videoserver
 * \date Oct 30, 2010
 * \author Christian Askeland
 */

class cxGrabber_EXPORT ImageServer: public QTcpServer
{
	Q_OBJECT

public:
	ImageServer(QObject* parent = NULL);
	virtual ~ImageServer();
	bool startListen(int port);
	static void printHelpText();
	static QString getArgumentHelpText(QString applicationName);
	bool initialize();
protected:
	void incomingConnection(qintptr socketDescriptor);
private slots:
	void socketDisconnectedSlot();
private:
	StreamerPtr mImageSender;
	QPointer<QTcpSocket> mSocket;
};

} // namespace cx

#endif /* CXIMAGESERVER_H_ */
