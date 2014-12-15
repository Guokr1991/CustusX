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

#include "cxTrackerConfigurationImpl.h"
#include "cxDataLocations.h"

#include "cxDefinitionStrings.h"
#include "cxToolConfigurationParser.h"
#include "cxFileHelpers.h"

namespace cx
{

void TrackerConfigurationImpl::saveConfiguration(const Configuration& config)
{
	ConfigurationFileParser::Configuration data;
	data.mFileName = config.mUid;
	data.mClinical_app = string2enum<CLINICAL_APPLICATION>(config.mClinicalApplication);

	QStringList selectedTools = config.mTools;
	QString referencePath = config.mReferenceTool;

	TRACKING_SYSTEM selectedTracker = string2enum<TRACKING_SYSTEM>(config.mTrackingSystem);

	ConfigurationFileParser::ToolFilesAndReferenceVector toolfilesAndRefVector;
//	QFile configFile(data.mFileName);
//	QFileInfo info(configFile);
//	QDir dir = info.dir();
	foreach(QString absoluteToolPath, selectedTools)
	{
//	  QString relativeToolFilePath = dir.relativeFilePath(absoluteToolPath);
	  ConfigurationFileParser::ToolFileAndReference tool;
//	  tool.first = relativeToolFilePath;
	  tool.first = absoluteToolPath;
	  tool.second = (absoluteToolPath == referencePath);
	  toolfilesAndRefVector.push_back(tool);
	}

	data.mTrackersAndTools[selectedTracker] = toolfilesAndRefVector;

	ConfigurationFileParser::saveConfiguration(data);
}

TrackerConfigurationImpl::Configuration TrackerConfigurationImpl::getConfiguration(QString uid)
{
	ConfigurationFileParser parser(uid);

	Configuration retval;
	retval.mUid = uid;
	retval.mName = QFileInfo(uid).completeBaseName();

	CLINICAL_APPLICATION application = parser.getApplicationapplication();
	retval.mClinicalApplication = enum2string(application);

	std::vector<IgstkTracker::InternalStructure> trackers = parser.getTrackers();
	for (unsigned i = 0; i < trackers.size(); ++i)
	{
		retval.mTrackingSystem = enum2string(trackers[i].mType);
		// only one trackingsystem is returned. (backed supports more than is needed.)
	}

	std::vector<QString> tools = parser.getAbsoluteToolFilePaths();
	for (unsigned i = 0; i < tools.size(); ++i)
	{
		retval.mTools << tools[i];
	}

	retval.mReferenceTool = parser.getAbsoluteReferenceFilePath();

	return retval;
}

QStringList TrackerConfigurationImpl::getToolsGivenFilter(QStringList applicationsFilter,
														  QStringList trackingsystemsFilter)
{
	QStringList allTools = this->getAllTools();
	QStringList filteredTools = this->filter(allTools, applicationsFilter, trackingsystemsFilter);
	return filteredTools;
}

TrackerConfigurationImpl::Tool TrackerConfigurationImpl::getTool(QString uid)
{
	Tool retval;
	retval.mUid = uid;

	QString absoluteFilePath = uid;
	QFile file(absoluteFilePath);
	QFileInfo info(file);
	retval.mName = info.dir().dirName();

	ToolFileParser parser(absoluteFilePath);
	IgstkTool::InternalStructure internal = parser.getTool();

	retval.mTrackingSystem = enum2string(internal.mTrackerType);
	retval.mIsReference = internal.mIsReference;
	retval.mPictureFilename = internal.mPictureFileName;

	return retval;
}

QStringList TrackerConfigurationImpl::filter(QStringList toolsToFilter, QStringList applicationsFilter,
		QStringList trackingsystemsFilter)
{
	QStringList retval;

	foreach(QString toolFilePath, toolsToFilter)
	{
		//get internal tool
		IgstkTool::InternalStructure internal = this->getToolInternal(toolFilePath);

		//check tracking systems
		QString trackerName = enum2string(internal.mTrackerType);
		if(!trackingsystemsFilter.contains(trackerName, Qt::CaseInsensitive))
		continue;

		//check applications
		bool passedApplicationFilter = false;
		std::vector<CLINICAL_APPLICATION>::iterator it = internal.mClinicalApplications.begin();
		while(it != internal.mClinicalApplications.end() && !passedApplicationFilter)
		{
			QString applicationName = enum2string(*it);
			if(applicationsFilter.contains(applicationName, Qt::CaseInsensitive))
			{
				passedApplicationFilter = true;
			}
			++it;
		}
		if(!passedApplicationFilter)
		continue;

		//add if filters passed
		retval << toolFilePath;
	}

	return retval;
}

IgstkTool::InternalStructure TrackerConfigurationImpl::getToolInternal(QString toolAbsoluteFilePath)
{
	IgstkTool::InternalStructure retval;

	ToolFileParser parser(toolAbsoluteFilePath);
	retval = parser.getTool();

	return retval;
}

bool TrackerConfigurationImpl::verifyTool(QString uid)
{
	IgstkTool::InternalStructure internal = this->getToolInternal(uid);
	return internal.verify();
}

QString TrackerConfigurationImpl::getConfigurationApplicationsPath(QString application)
{
	QStringList path;
	path << DataLocations::getRootConfigPath() << "tool" << application;
	return path.join("/");
}

QStringList TrackerConfigurationImpl::getConfigurationsGivenApplication(QString application)
{
	QStringList retval;

	QStringList configPaths = DataLocations::getRootConfigPaths();

	for (int i=0; i< configPaths.size(); ++i)
	{
		QDir dir(configPaths[i]+"/tool/"+application);
		retval << cx::getAbsolutePathToXmlFiles(dir.absolutePath());
	}
	return retval;
}

QStringList TrackerConfigurationImpl::getAllConfigurations()
{
	QStringList retval;
	QStringList rootPaths = DataLocations::getRootConfigPaths();

	for (int i=0; i< rootPaths.size(); ++i)
	{
		QString configFilePath = rootPaths[i] + "/tool/";
		foreach(QFileInfo dir, cx::getDirs(configFilePath))
		{
			retval << cx::getAbsolutePathToXmlFiles(dir.absoluteFilePath());
		}
	}
	return retval;
}

QStringList TrackerConfigurationImpl::getAllTools()
{
	bool includeSubDirs = true;
	QStringList toolFilePath = cx::DataLocations::getToolsPaths();
	QStringList retval;
	for (int i=0; i<toolFilePath.size(); ++i)
		retval << getAbsolutePathToXmlFiles(toolFilePath[i], includeSubDirs);
	return retval;
}

QStringList TrackerConfigurationImpl::getSupportedTrackingSystems()
{
	QStringList retval;
	retval = IgstkTracker::getSupportedTrackingSystems();
	return retval;
}


} // namespace cx

