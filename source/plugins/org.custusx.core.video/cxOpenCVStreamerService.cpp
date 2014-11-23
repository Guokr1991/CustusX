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
#include "cxOpenCVStreamerService.h"

#include "cxStringDataAdapterXml.h"
#include "cxDoubleDataAdapterXml.h"
#include "cxBoolDataAdapterXml.h"
#include "cxIGTLinkClientStreamer.h"
#include "cxImageStreamerOpenCV.h"
#include "cxUtilHelpers.h"
#include "cxReporter.h"
#include "QApplication"
#include <QDir>
#include "cxDataLocations.h"
#include "cxLocalServerStreamerServer.h"

namespace cx
{

QString OpenCVStreamerService::getName()
{
	return "OpenCV";
}

std::vector<DataAdapterPtr> OpenCVStreamerService::getSettings(QDomElement root)
{
	std::vector<DataAdapterPtr> retval;
	std::vector<DataAdapterPtr> opencvArgs = ImageStreamerOpenCVArguments().getSettings(root);
	std::copy(opencvArgs.begin(), opencvArgs.end(), back_inserter(retval));

	std::vector<DataAdapterPtr> localsvrArgs = LocalServerStreamerArguments().getSettings(root);
	std::copy(localsvrArgs.begin(), localsvrArgs.end(), back_inserter(retval));

	return retval;
}

BoolDataAdapterPtr OpenCVStreamerService::getRunLocalServerOption(QDomElement root)
{
	BoolDataAdapterXmlPtr retval;
	bool defaultValue = false;
	retval = BoolDataAdapterXml::initialize("runlocalserver", "Run Local Server",
											"Run streamer in a separate process",
											defaultValue, root);
	retval->setAdvanced(false);
	retval->setGroup("Connection");
	return retval;
}
StringDataAdapterPtr OpenCVStreamerService::getLocalServerNameOption(QDomElement root)
{
	StringDataAdapterXmlPtr retval;
	QString defaultValue = "OpenIGTLinkServer";
	retval = StringDataAdapterXml::initialize("localservername", "Server Name",
											  "Name of server executable, used only if Run Local Server is set.",
											  defaultValue, root);
	retval->setGuiRepresentation(StringDataAdapter::grFILENAME);
	retval->setAdvanced(false);
	retval->setGroup("Connection");
	return retval;
}

StreamerPtr OpenCVStreamerService::createStreamer(QDomElement root)
{
	StringMap args = ImageStreamerOpenCVArguments().convertToCommandLineArguments(root);
	StreamerPtr localServerStreamer = LocalServerStreamer::createStreamerIfEnabled(root, args);
	if (localServerStreamer)
	{
		return localServerStreamer;
	}

	else
	{
		boost::shared_ptr<ImageStreamerOpenCV> streamer(new ImageStreamerOpenCV());
		streamer->initialize(args);
		return streamer;
	}
}

} // namespace cx