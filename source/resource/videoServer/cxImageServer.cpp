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

#include "cxImageServer.h"

#include "cxLogger.h"
#include "cxTypeConversions.h"
#include <iostream>
#include <QCoreApplication>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTcpSocket>
#include "cxCommandlineImageStreamerFactory.h"
//#include "cxSender.h"
#include "cxGrabberSenderQTcpSocket.h"

namespace cx
{

ImageServer::ImageServer(QObject* parent) :
	QTcpServer(parent)
{}

bool ImageServer::initialize()
{
	bool ok = false;

	StringMap args = cx::extractCommandlineOptions(QCoreApplication::arguments());
	mImageSender = CommandlineImageStreamerFactory().getFromArguments(args);
	if(!mImageSender)
		return false;

	ok = true;

	return ok;
}

bool ImageServer::startListen(int port)
{
	bool started = this->listen(QHostAddress::Any, port);

	if (started)
	{
		//Find IP adresses
		std::cout <<  "Server IP adresses: " << std::endl;
		foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
		{
			if (interface.flags().testFlag(QNetworkInterface::IsRunning))
			foreach (QNetworkAddressEntry entry, interface.addressEntries())
			{
				if ( interface.hardwareAddress() != "00:00:00:00:00:00" && entry.ip().toString() != "127.0.0.1" && entry.ip().toString().contains(".") )
				std::cout << string_cast(interface.name()) << " " << entry.ip().toString() << std::endl;
			}
		}

		std::cout << QString("Server is listening to port %2").arg(this->serverPort()).toStdString() << std::endl;
		return true;
	}
	else
	{
		std::cout << "Server failed to start. Error: " << this->errorString().toStdString() << std::endl;
		return false;
	}
}

ImageServer::~ImageServer()
{
}

void ImageServer::incomingConnection(qintptr socketDescriptor)
{
	std::cout << "Server: Incoming connection..." << std::endl;

	if (mSocket != 0)
	{
		reportError("The image server can only handle a single connection.");
		return;
	}

	mSocket = new QTcpSocket();
	connect(mSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnectedSlot()));
	mSocket->setSocketDescriptor(socketDescriptor);
	QString clientName = mSocket->localAddress().toString();
	report("Connected to "+clientName+". Session started.");
	SenderPtr sender(new GrabberSenderQTcpSocket(mSocket));

	mImageSender->startStreaming(sender);
}

void ImageServer::socketDisconnectedSlot()
{
	if (mImageSender)
		mImageSender->stopStreaming();

	if (mSocket)
	{
		QString clientName = mSocket->localAddress().toString();
		report("Disconnected from "+clientName+". Session ended.");
		mSocket->deleteLater();
	}
}

void ImageServer::printHelpText()
{
	std::cout << getArgumentHelpText(qApp->applicationName());
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Press Ctrl + C to close the server."<< std::endl;
	std::cout << std::endl;
}

QString ImageServer::getArgumentHelpText(QString applicationName)
{
	std::stringstream ss;
	cx::CommandlineImageStreamerFactory factory;

	ss << "Usage: " << applicationName << " (--arg <argval>)*" << std::endl;
	ss << "    --port   : Tcp/IP port # (default=18333)" << std::endl;
	ss << "    --type   : Grabber type  (default=" << factory.getDefaultSenderType().toStdString() << ")"
		<< std::endl;
	ss << std::endl;
	ss << "    Select one of the types below:" << std::endl;

	QStringList types = factory.getSenderTypes();
	for (int i = 0; i < types.size(); ++i)
	{
		QStringList args = factory.getArgumentDescription(types[i]);
		ss << std::endl;
		ss << "      type = " << types[i].toStdString() << std::endl;
		for (int j = 0; j < args.size(); ++j)
			ss << "        " << args[j].toStdString() << std::endl;
	}
	return qstring_cast(ss.str());
}

}
