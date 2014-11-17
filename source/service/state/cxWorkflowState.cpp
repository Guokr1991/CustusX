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

#include "cxWorkflowState.h"
#include "cxVideoConnectionManager.h"
#include "cxDataManager.h"
#include "cxStateService.h"
#include "cxPatientData.h"
#include "cxSettings.h"
#include "cxTrackingService.h"
#include "cxTool.h"
#include "cxPatientService.h"
#include "cxVideoServiceOld.h"
#include "cxStateServiceBackend.h"
#include "cxPatientModelService.h"

namespace cx
{

void WorkflowState::onEntry(QEvent * event)
{
	report("Workflow change to [" + mName + "]");
	if (mAction)
		mAction->setChecked(true);
}

void WorkflowState::onExit(QEvent * event)
{
	emit aboutToExit();
}

std::vector<WorkflowState*> WorkflowState::getChildStates()
{
	QObjectList childrenList = this->children();
	std::vector<WorkflowState*> retval;
	for (int i = 0; i < childrenList.size(); ++i)
	{
		WorkflowState* state = dynamic_cast<WorkflowState*>(childrenList[i]);
		if (state)
			retval.push_back(state);
	}
	return retval;
}

QAction* WorkflowState::createAction(QActionGroup* group)
{
	if (mAction)
		return mAction;

	mAction = new QAction(this->getName(), group);
	mAction->setIcon(this->getIcon());
	mAction->setStatusTip(this->getName());
	mAction->setCheckable(true);
	mAction->setData(QVariant(this->getUid()));
	this->canEnterSlot();

	connect(mAction, SIGNAL(triggered()), this, SLOT(setActionSlot()));

	return mAction;
}

void WorkflowState::canEnterSlot()
{
	if (mAction)
		mAction->setEnabled(this->canEnter());
}

void WorkflowState::setActionSlot()
{
	this->machine()->postEvent(new RequestEnterStateEvent(this->getUid()));
}


void WorkflowState::autoStartHardware()
{
	if (settings()->value("Automation/autoStartTracking").toBool())
		mBackend->getToolManager()->setState(Tool::tsTRACKING);
	if (settings()->value("Automation/autoStartStreaming").toBool())
		mBackend->getVideoServiceOld()->getVideoConnection()->launchAndConnectServer(mVideoService);
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

NavigationWorkflowState::NavigationWorkflowState(VideoServicePtr videoService, QState* parent, StateServiceBackendPtr backend) :
				WorkflowState(videoService, parent, "NavigationUid", "Navigation", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

void NavigationWorkflowState::onEntry(QEvent * event)
{
	this->autoStartHardware();
}

bool NavigationWorkflowState::canEnter() const
{
	return mBackend->getPatientService()->isPatientValid();
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

RegistrationWorkflowState::RegistrationWorkflowState(VideoServicePtr videoService, QState* parent, StateServiceBackendPtr backend) :
				WorkflowState(videoService, parent, "RegistrationUid", "Registration", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}
;

bool RegistrationWorkflowState::canEnter() const
{
// We need to perform patient orientation prior to
// running and us acq. Thus we need access to the reg mode.
	return mBackend->getPatientService()->isPatientValid();
}
;

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

PreOpPlanningWorkflowState::PreOpPlanningWorkflowState(VideoServicePtr videoService, QState* parent, StateServiceBackendPtr backend) :
				WorkflowState(videoService, parent, "PreOpPlanningUid", "Preoperative Planning", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(dataAddedOrRemoved()), this, SLOT(canEnterSlot()));
}

bool PreOpPlanningWorkflowState::canEnter() const
{
	return !mBackend->getPatientService()->getData().empty();
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

IntraOpImagingWorkflowState::IntraOpImagingWorkflowState(VideoServicePtr videoService, QState* parent, StateServiceBackendPtr backend) :
				WorkflowState(videoService, parent, "IntraOpImagingUid", "Intraoperative Imaging", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

void IntraOpImagingWorkflowState::onEntry(QEvent * event)
{
	this->autoStartHardware();
}

bool IntraOpImagingWorkflowState::canEnter() const
{
	return mBackend->getPatientService()->isPatientValid();
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

PostOpControllWorkflowState::PostOpControllWorkflowState(VideoServicePtr videoService, QState* parent, StateServiceBackendPtr backend) :
				WorkflowState(videoService, parent, "PostOpControllUid", "Postoperative Control", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(dataAddedOrRemoved()), this, SLOT(canEnterSlot()));
}

bool PostOpControllWorkflowState::canEnter() const
{
	return !mBackend->getPatientService()->getData().empty();
}

}

