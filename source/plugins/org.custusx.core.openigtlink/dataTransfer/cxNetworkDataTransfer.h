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
#ifndef CXNETWORKDATATRANSFER_H_
#define CXNETWORKDATATRANSFER_H_

#include "cxXmlOptionItem.h"
#include "cxForwardDeclarations.h"

class ctkPluginContext;

namespace cx {

class NetworkConnection;
typedef boost::shared_ptr<class BoolProperty> BoolPropertyPtr;
typedef boost::shared_ptr<class StringProperty> StringPropertyPtr;
typedef boost::shared_ptr<class StringPropertySelectData> StringPropertySelectDataPtr;
typedef boost::shared_ptr<class NetworkConnectionHandle> NetworkConnectionHandlePtr;


/**
 * Handle transfer of data to and from CustusX over OpenIGTLink.
 * The connection must be set up and is available from getOpenIGTLink().
 *
 * When AcceptIncomingData is set and the system is connected to a server,
 * incoming data is automatically added to PatientModel and possibly shown
 * in the Views.
 *
 * Use the DataToSend and onSend() to send data to server.
 */
class NetworkDataTransfer : public QObject
{
	Q_OBJECT
public:
	NetworkDataTransfer(ctkPluginContext* context, NetworkConnectionHandlePtr connection, QObject *parent=NULL);
	~NetworkDataTransfer();

	BoolPropertyPtr getAcceptIncomingData() { return mAcceptIncomingData; }
	BoolPropertyPtr getStreamActiveVideoSource() { return mStreamActiveVideoSource; }
	StringPropertySelectDataPtr getDataToSend() { return mDataToSend; }
	PatientModelServicePtr getPatientModelService() { return mPatientModelService; }
	ViewServicePtr getViewService() { return mViewService; }
	VideoServicePtr getVideoService() { return mVideoService; }
	NetworkConnectionHandlePtr getOpenIGTLink();

	/**
	 * Send data over igtl, using the data set in the DataToSend property
	 */
	void onSend();

private:
	BoolPropertyPtr mStreamActiveVideoSource;
	BoolPropertyPtr mAcceptIncomingData;
	StringPropertySelectDataPtr mDataToSend;
	XmlOptionFile mOptions;
	ctkPluginContext* mContext;
	NetworkConnectionHandlePtr mOpenIGTLink;

	PatientModelServicePtr mPatientModelService;
	ViewServicePtr mViewService;
	VideoServicePtr mVideoService;

	VideoSourcePtr mStreamingVideoSource;

	QString getConfigUid() const;
	void onImageReceived(ImagePtr image);
	void onMeshReceived(MeshPtr image);
	void onDataReceived(DataPtr data);

	void onNewStreamFrame();

	void onStreamActiveVideoSourceChanged();
	/**
	 * Stream data over igtl, using the active stream (experimental)
	 */
	void startStream();
	void stopStream();

};

} // namespace cx

#endif // CXNETWORKDATATRANSFER_H_
