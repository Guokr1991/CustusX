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
#ifndef CXIGTLINKCLIENTSTREAMER_H
#define CXIGTLINKCLIENTSTREAMER_H

#include "cxStreamer.h"
#include "org_custusx_core_video_Export.h"
#include <QAbstractSocket>
#include "cxIGTLinkImageMessage.h"
#include "cxIGTLinkUSStatusMessage.h"

class QTcpSocket;

namespace cx
{

/**
 * Streamer that listens to an IGTLink connection, then
 * streams the incoming data.
 *
 * \addtogroup org_custusx_core_video
 * \author Christian Askeland, SINTEF
 * \date 2014-11-20
 */
class org_custusx_core_video_EXPORT IGTLinkClientStreamer: public Streamer
{
Q_OBJECT

public:
	IGTLinkClientStreamer();
	virtual ~IGTLinkClientStreamer();

	void setAddress(QString address, int port);

	virtual bool startStreaming(SenderPtr sender);
	virtual void stopStreaming();

	virtual QString getType();

private slots:
//	virtual void myStreamSlot();
	virtual void streamSlot() {}
private slots:
	void readyReadSlot();

	void hostFoundSlot();
	void connectedSlot();
	void disconnectedSlot();
	void errorSlot(QAbstractSocket::SocketError);

private:
	SenderPtr mSender;

	virtual QString hostDescription() const; // threadsafe
	bool ReceiveImage(QTcpSocket* socket, igtl::MessageHeader::Pointer& header);
	bool ReceiveSonixStatus(QTcpSocket* socket, igtl::MessageHeader::Pointer& header);
	bool readOneMessage();
	void addToQueue(IGTLinkUSStatusMessage::Pointer msg);
	void addToQueue(IGTLinkImageMessage::Pointer msg);
	bool multipleTryConnectToHost();
	bool tryConnectToHost();

	bool mHeadingReceived;
	QString mAddress;
	int mPort;
	boost::shared_ptr<QTcpSocket> mSocket;
	igtl::MessageHeader::Pointer mHeaderMsg;
	IGTLinkUSStatusMessage::Pointer mUnsentUSStatusMessage; ///< received message, will be added to queue when next image arrives


};
typedef boost::shared_ptr<class IGTLinkClientStreamer> IGTLinkClientStreamerPtr;

} // namespace cx

#endif // CXIGTLINKCLIENTSTREAMER_H
