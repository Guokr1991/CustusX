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

#include "cxUSAcquisition.h"

#include "cxBoolDataAdapterXml.h"

#include "cxSettings.h"
#include "cxVideoService.h"
#include "cxTrackingService.h"
#include "cxUSSavingRecorder.h"
#include "cxAcquisitionData.h"
#include "cxUsReconstructionService.h"
#include "cxUSReconstructInputData.h"
#include "cxPatientModelService.h"
#include "cxVideoSource.h"
#include "cxAcquisitionService.h"
#include "cxUsReconstructionService.h"


namespace cx
{

USAcquisition::USAcquisition(AcquisitionPtr base, UsReconstructionServicePtr reconstructer, QObject* parent) :
	QObject(parent),
	mBase(base),
	mUsReconstructionService(reconstructer)
{
	mCore.reset(new USSavingRecorder());
	connect(mCore.get(), SIGNAL(saveDataCompleted(QString)), this, SLOT(checkIfReadySlot()));
	connect(mCore.get(), SIGNAL(saveDataCompleted(QString)), this, SIGNAL(saveDataCompleted(QString)));


	connect(trackingService().get(), &TrackingService::stateChanged, this, &USAcquisition::checkIfReadySlot);
	connect(trackingService().get(), SIGNAL(dominantToolChanged(const QString&)), this, SLOT(checkIfReadySlot()));
	connect(videoService().get(), SIGNAL(activeVideoSourceChanged()), this, SLOT(checkIfReadySlot()));
	connect(videoService().get(), &VideoService::connected, this, &USAcquisition::checkIfReadySlot);

	connect(mBase.get(), SIGNAL(started()), this, SLOT(recordStarted()));
	connect(mBase.get(), SIGNAL(acquisitionStopped()), this, SLOT(recordStopped()), Qt::QueuedConnection);
	connect(mBase.get(), SIGNAL(cancelled()), this, SLOT(recordCancelled()));

	this->checkIfReadySlot();
}

USAcquisition::~USAcquisition()
{

}

void USAcquisition::checkIfReadySlot()
{
	bool tracking = trackingService()->getState()>=Tool::tsTRACKING;
	bool streaming = videoService()->isConnected();
	ToolPtr tool = trackingService()->getFirstProbe();

	QString mWhatsMissing;
	mWhatsMissing.clear();

	if(tracking && streaming)
	{
		mWhatsMissing = "<font color=green>Ready to record!</font><br>";
		if (!tool || !tool->getVisible())
		{
			mWhatsMissing += "<font color=orange>Probe not visible</font><br>";
		}
	}
	else
	{
		if(!tracking)
			mWhatsMissing.append("<font color=red>Need to start tracking.</font><br>");
		if(!streaming)
			mWhatsMissing.append("<font color=red>Need to start streaming.</font><br>");
	}

	int saving = mCore->getNumberOfSavingThreads();

	if (saving!=0)
		mWhatsMissing.append(QString("<font color=orange>Saving %1 acquisition data.</font><br>").arg(saving));

	// remove redundant line breaks
	QStringList list = mWhatsMissing.split("<br>", QString::SkipEmptyParts);
	mWhatsMissing = list.join("<br>");

	//Make sure we have at least 2 lines to avoid "bouncing buttons"
	if (list.size() < 2)
		mWhatsMissing.append("<br>");

	// do not require tracking to be present in order to perform an acquisition.
    //mBase->setReady(streaming, mWhatsMissing);
    mBase->setReady(true, mWhatsMissing);
}

int USAcquisition::getNumberOfSavingThreads() const
{
	return mCore->getNumberOfSavingThreads();
}

void USAcquisition::recordStarted()
{
	mUsReconstructionService->selectData(USReconstructInputData()); // clear old data in reconstructeer

	ToolPtr tool = trackingService()->getFirstProbe();
	mCore->setWriteColor(this->getWriteColor());
	mCore->startRecord(mBase->getLatestSession(), tool, this->getRecordingVideoSources(tool));
}

void USAcquisition::recordStopped()
{
	mCore->stopRecord();

	this->sendAcquisitionDataToReconstructer();

	mCore->set_rMpr(patientService()->get_rMpr());
	bool compress = settings()->value("Ultrasound/CompressAcquisition", true).toBool();
	QString baseFolder = patientService()->getActivePatientFolder();
	mCore->startSaveData(baseFolder, compress);

	mCore->clearRecording();

	this->checkIfReadySlot();
}

void USAcquisition::recordCancelled()
{
	mCore->cancelRecord();
}

void USAcquisition::sendAcquisitionDataToReconstructer()
{
	mCore->set_rMpr(patientService()->get_rMpr());

	VideoSourcePtr activeVideoSource = videoService()->getActiveVideoSource();
	if (activeVideoSource)
	{
		USReconstructInputData data = mCore->getDataForStream(activeVideoSource->getUid());
		mUsReconstructionService->selectData(data);
		emit acquisitionDataReady();
	}
}

std::vector<VideoSourcePtr> USAcquisition::getRecordingVideoSources(ToolPtr tool)
{
	std::vector<VideoSourcePtr> retval;

	if (tool && tool->getProbe())
	{
		ProbePtr probe = tool->getProbe();
		QStringList sources = probe->getAvailableVideoSources();
		for (unsigned i=0; i<sources.size(); ++i)
			retval.push_back(probe->getRTSource(sources[i]));
	}
	else
	{
		retval = videoService()->getVideoSources();
	}

	return retval;
}

bool USAcquisition::getWriteColor() const
{
	DataAdapterPtr angio = mUsReconstructionService->getParam("Angio data");
	bool writeColor = angio->getValueAsVariant().toBool()
	        ||  !settings()->value("Ultrasound/8bitAcquisitionData").toBool();
	return writeColor;
}

}
