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


#ifndef CXOPENIGTLINKCLIENT_H
#define CXOPENIGTLINKCLIENT_H

#include "org_custusx_core_tracking_system_openigtlink_Export.h"

#include <QObject>
#include "igtlMessageHeader.h"
#include "cxSocket.h"
#include "cxTransform3D.h"
#include "cxImage.h"

namespace cx {

class org_custusx_core_tracking_system_openigtlink_EXPORT OpenIGTLinkClient : public QObject
{
    Q_OBJECT
public:

    explicit OpenIGTLinkClient(QObject *parent = 0);

public slots:
    void setIpAndPort(QString ip, int port=18944); //not threadsafe
    void requestConnect();
    void requestDisconnect();

signals:
    void connected();
    void disconnected();
    void error();

    void transform(QString devicename, Transform3D transform, double timestamp);
    void image(ImagePtr image);

private slots:
    void internalConnected();
    void internalDisconnected();
    void internalDataAvailable();

private:
    bool socketIsConnected();
    bool enoughBytesAvailableOnSocket(int bytes) const;
    bool receiveHeader(const igtl::MessageHeader::Pointer header) const;
    bool receiveBody(const igtl::MessageHeader::Pointer header);

    bool receiveTransform(const igtl::MessageBase::Pointer header);
    bool receiveImage(const igtl::MessageBase::Pointer header);
    bool receiveStatus(const igtl::MessageBase::Pointer header);
    bool receiveString(const igtl::MessageBase::Pointer header);

    bool socketReceive(void *packPointer, int packSize) const;
    void checkCRC(int c) const;

    SocketPtr mSocket;
    igtl::MessageHeader::Pointer mHeader;
    bool mHeaderReceived;
    QString mIp;
    int mPort;
};

} //namespace cx

#endif // CXOPENIGTLINKCLIENT_H
