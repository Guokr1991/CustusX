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

#include "cxRegistrationImplService.h"

#include <ctkPluginContext.h>
#include <ctkServiceTracker.h>
#include <vtkPoints.h>
#include <vtkLandmarkTransform.h>
#include <vtkMatrix4x4.h>

#include "cxData.h"
#include "cxTypeConversions.h"
#include "cxReporter.h"
#include "cxRegistrationTransform.h"
#include "cxFrameForest.h"
#include "cxPatientModelService.h"
#include "cxRegistrationApplicator.h"
#include "cxLandmark.h"
#include "cxPatientModelServiceProxy.h"
#include "cxLandmarkTranslationRegistration.h"
#include "cxSessionStorageServiceProxy.h"
#include "cxXmlNodeWrapper.h"

namespace cx
{

RegistrationImplService::RegistrationImplService(ctkPluginContext *context) :
	mLastRegistrationTime(QDateTime::currentDateTime()),
	mContext(context),
	mPatientModelService(new PatientModelServiceProxy(context)),
	mSession(SessionStorageServiceProxy::create(context))
{
//	mLastRegistrationTime = QDateTime::currentDateTime();

	connect(mSession.get(), &SessionStorageService::cleared, this, &RegistrationImplService::clearSlot);
	connect(mSession.get(), &SessionStorageService::isLoading, this, &RegistrationImplService::duringLoadPatientSlot);
	connect(mSession.get(), &SessionStorageService::isSaving, this, &RegistrationImplService::duringSavePatientSlot);

//	connect(mPatientModelService.get(), &PatientModelService::isSaving, this, &RegistrationImplService::duringSavePatientSlot);
//	connect(mPatientModelService.get(), &PatientModelService::isLoading, this, &RegistrationImplService::duringLoadPatientSlot);
//	connect(mPatientModelService.get(), &PatientModelService::cleared, this, &RegistrationImplService::clearSlot);
}

RegistrationImplService::~RegistrationImplService()
{
}

void RegistrationImplService::duringSavePatientSlot(QDomElement& node)
{
	XMLNodeAdder root(node);
	QDomElement managerNode = root.descend("managers").node().toElement();
	this->addXml(managerNode);
//	QDomElement managerNode = mPatientModelService->getCurrentWorkingElement("managers");
//	this->addXml(managerNode);
}

void RegistrationImplService::duringLoadPatientSlot(QDomElement& node)
{
	XMLNodeParser root(node);
	QDomElement registrationManager = root.descend("managers/registrationManager").node().toElement();
	if (!registrationManager.isNull())
		this->parseXml(registrationManager);

//	QDomElement registrationManager = mPatientModelService->getCurrentWorkingElement("managers/registrationManager");
//	this->parseXml(registrationManager);
}

void RegistrationImplService::addXml(QDomNode& parentNode)
{
	QDomDocument doc = parentNode.ownerDocument();
	QDomElement base = doc.createElement("registrationManager");
	parentNode.appendChild(base);

	QDomElement fixedDataNode = doc.createElement("fixedDataUid");
	DataPtr fixedData = this->getFixedData();
	if(fixedData)
	{
		fixedDataNode.appendChild(doc.createTextNode(fixedData->getUid()));
	}
	base.appendChild(fixedDataNode);

	QDomElement movingDataNode = doc.createElement("movingDataUid");
	DataPtr movingData = this->getMovingData();
	if(movingData)
	{
		movingDataNode.appendChild(doc.createTextNode(movingData->getUid()));
	}
	base.appendChild(movingDataNode);
}

void RegistrationImplService::parseXml(QDomNode& dataNode)
{
	QString fixedData = dataNode.namedItem("fixedDataUid").toElement().text();
	this->setFixedData(mPatientModelService->getData(fixedData));

	QString movingData = dataNode.namedItem("movingDataUid").toElement().text();
	this->setMovingData(mPatientModelService->getData(movingData));
}

void RegistrationImplService::clearSlot()
{
	this->setLastRegistrationTime(QDateTime());
	this->setFixedData(DataPtr());
}



void RegistrationImplService::setMovingData(DataPtr movingData)
{
  mMovingData = movingData;
  emit movingDataChanged( (mMovingData) ? qstring_cast(mMovingData->getUid()) : "");
}

void RegistrationImplService::setFixedData(DataPtr fixedData)
{
  if(mFixedData == fixedData)
	return;

  mFixedData = fixedData;
  if (mFixedData)
	report("Registration fixed data set to "+mFixedData->getUid());
  emit fixedDataChanged( (mFixedData) ? qstring_cast(mFixedData->getUid()) : "");
}

DataPtr RegistrationImplService::getMovingData()
{
  return mMovingData;
}

DataPtr RegistrationImplService::getFixedData()
{
  return mFixedData;
}

QDateTime RegistrationImplService::getLastRegistrationTime()
{
	return mLastRegistrationTime;
}


void RegistrationImplService::setLastRegistrationTime(QDateTime time)
{
	mLastRegistrationTime = time;
}

void RegistrationImplService::doPatientRegistration()
{
	DataPtr fixedImage = this->getFixedData();

	if(!fixedImage)
	{
	reportError("The fixed data is not set, cannot do patient registration!");
		return;
	}
	LandmarkMap fixedLandmarks = fixedImage->getLandmarks()->getLandmarks();
	LandmarkMap toolLandmarks = mPatientModelService->getPatientLandmarks()->getLandmarks();

	this->writePreLandmarkRegistration(fixedImage->getName(), fixedImage->getLandmarks()->getLandmarks());
	this->writePreLandmarkRegistration("physical", toolLandmarks);

	std::vector<QString> landmarks = this->getUsableLandmarks(fixedLandmarks, toolLandmarks);

	vtkPointsPtr p_ref = this->convertTovtkPoints(landmarks, fixedLandmarks, fixedImage->get_rMd());
	vtkPointsPtr p_pr = this->convertTovtkPoints(landmarks, toolLandmarks, Transform3D::Identity());

	// ignore if too few data.
	if (p_ref->GetNumberOfPoints() < 3)
		return;

	bool ok = false;
	Transform3D rMpr = this->performLandmarkRegistration(p_pr, p_ref, &ok);
	if (!ok)
	{
		reportError("P-I Landmark registration: Failed to register: [" + qstring_cast(p_pr->GetNumberOfPoints()) + "p]");
		return;
	}

	this->applyPatientRegistration(rMpr, "Patient Landmark");
}

void RegistrationImplService::doFastRegistration_Translation()
{
	DataPtr fixedImage = this->getFixedData();
	if(!fixedImage)
	{
	reportError("The fixed data is not set, cannot do image registration!");
		return;
	}

	LandmarkMap fixedLandmarks = fixedImage->getLandmarks()->getLandmarks();
	LandmarkMap toolLandmarks = mPatientModelService->getPatientLandmarks()->getLandmarks();

	this->writePreLandmarkRegistration(fixedImage->getName(), fixedImage->getLandmarks()->getLandmarks());
	this->writePreLandmarkRegistration("physical", toolLandmarks);

	std::vector<QString> landmarks = this->getUsableLandmarks(fixedLandmarks, toolLandmarks);

	Transform3D rMd = fixedImage->get_rMd();
	Transform3D rMpr_old = mPatientModelService->get_rMpr();
	std::vector<Vector3D> p_pr_old = this->convertAndTransformToPoints(landmarks, fixedLandmarks, rMpr_old.inv()*rMd);
	std::vector<Vector3D> p_pr_new = this->convertAndTransformToPoints(landmarks, toolLandmarks, Transform3D::Identity());

	// ignore if too few data.
	if (p_pr_old.size() < 1)
		return;

	LandmarkTranslationRegistration landmarkTransReg;
	bool ok = false;
	Transform3D pr_oldMpr_new = landmarkTransReg.registerPoints(p_pr_old, p_pr_new, &ok);
	if (!ok)
	{
		reportError("Fast translation registration: Failed to register: [" + qstring_cast(p_pr_old.size()) + "points]");
		return;
	}

	this->applyPatientRegistration(rMpr_old*pr_oldMpr_new, "Fast Translation");
}

/**Perform a fast orientation by setting the patient registration equal to the current dominant
 * tool position.
 * Input is an additional transform tMtm that modifies the tool position. Use this to
 * define DICOM-ish spaces relative to the tool.
 *
 */
void RegistrationImplService::doFastRegistration_Orientation(const Transform3D& tMtm, const Transform3D& prMt)
{
	//create a marked(m) space tm, which is related to tool space (t) as follows:
	//the tool is defined in DICOM space such that
	//the tool points toward the patients feet and the spheres faces the same
	//direction as the nose
		Transform3D tMpr = prMt.inv();

	Transform3D tmMpr = tMtm * tMpr;

	this->applyPatientRegistration(tmMpr, "Fast Orientation");

	// also apply the fast translation registration if any (this frees us form doing stuff in a well-defined order.)
	this->doFastRegistration_Translation();
}

std::vector<Vector3D> RegistrationImplService::convertAndTransformToPoints(const std::vector<QString>& uids, const LandmarkMap& data, Transform3D M)
{
	std::vector<Vector3D> retval;

	for (unsigned i=0; i<uids.size(); ++i)
	{
		QString uid = uids[i];
		Vector3D p = M.coord(data.find(uid)->second.getCoord());
		retval.push_back(p);
	}
	return retval;
}

/**Inspect the landmarks in data a and b, find landmarks defined in both of them and
 * that also is active.
 * Return the uids of these landmarks.
 */
std::vector<QString> RegistrationImplService::getUsableLandmarks(const LandmarkMap& data_a, const LandmarkMap& data_b)
{
	std::vector<QString> retval;
	std::map<QString, LandmarkProperty> props = mPatientModelService->getLandmarkProperties();
	std::map<QString, LandmarkProperty>::iterator iter;

	for (iter=props.begin(); iter!=props.end(); ++iter)
	{
		QString uid = iter->first;
		if (data_a.count(uid) && data_b.count(uid) && iter->second.getActive())
			retval.push_back(uid);
	}
	return retval;
}

void RegistrationImplService::doImageRegistration(bool translationOnly)
{
	//check that the fixed data is set
	DataPtr fixedImage = this->getFixedData();
	if(!fixedImage)
	{
	reportError("The fixed data is not set, cannot do landmark image registration!");
		return;
	}

	//check that the moving data is set
	DataPtr movingImage = this->getMovingData();
	if(!movingImage)
	{
	reportError("The moving data is not set, cannot do landmark image registration!");
		return;
	}

	// ignore self-registration, this gives no effect bestcase, buggy behaviour worstcase (has been observed)
	if(movingImage==fixedImage)
	{
		reportError("The moving and fixed are equal, ignoring landmark image registration!");
		return;
	}

	LandmarkMap fixedLandmarks = fixedImage->getLandmarks()->getLandmarks();
	LandmarkMap imageLandmarks = movingImage->getLandmarks()->getLandmarks();

	this->writePreLandmarkRegistration(fixedImage->getName(), fixedImage->getLandmarks()->getLandmarks());
	this->writePreLandmarkRegistration(movingImage->getName(), movingImage->getLandmarks()->getLandmarks());

	std::vector<QString> landmarks = getUsableLandmarks(fixedLandmarks, imageLandmarks);
	vtkPointsPtr p_fixed_r = convertTovtkPoints(landmarks, fixedLandmarks, fixedImage->get_rMd());
	vtkPointsPtr p_moving_r = convertTovtkPoints(landmarks, imageLandmarks, movingImage->get_rMd());

	int minNumberOfPoints = 3;
	if (translationOnly)
		minNumberOfPoints = 1;

	// ignore if too few data.
	if (p_fixed_r->GetNumberOfPoints() < minNumberOfPoints)
	{
		reportError(
			QString("Found %1 corresponding landmarks, need %2, cannot do landmark image registration!")
			.arg(p_fixed_r->GetNumberOfPoints())
			.arg(minNumberOfPoints)
			);
		return;
	}

	bool ok = false;
	QString idString;
	Transform3D delta;

	if (translationOnly)
	{
		LandmarkTranslationRegistration landmarkTransReg;
		delta = landmarkTransReg.registerPoints(convertVtkPointsToPoints(p_fixed_r), convertVtkPointsToPoints(p_moving_r), &ok);
		idString = QString("Image to Image Landmark Translation");
	}
	else
	{
		Transform3D rMd;
		delta = this->performLandmarkRegistration(p_moving_r, p_fixed_r, &ok);
		idString = QString("Image to Image Landmark");
	}

	if (!ok)
	{
		reportError("I-I Landmark registration: Failed to register: [" + qstring_cast(p_moving_r->GetNumberOfPoints()) + "p], "+ movingImage->getName());
		return;
	}

	this->applyImage2ImageRegistration(delta, idString);
}

std::vector<Vector3D> RegistrationImplService::convertVtkPointsToPoints(vtkPointsPtr base)
{
	std::vector<Vector3D> retval;

	for (int i=0; i<base->GetNumberOfPoints(); ++i)
	{
		Vector3D p(base->GetPoint(i));
		retval.push_back(p);
	}
	return retval;
}

/** Perform a landmark registration between the data sets source and target.
 *  Return transform from source to target.
 */
Transform3D RegistrationImplService::performLandmarkRegistration(vtkPointsPtr source, vtkPointsPtr target, bool* ok) const
{
	*ok = false;

	// too few data samples: ignore
	if (source->GetNumberOfPoints() < 3)
	{
		return Transform3D::Identity();
	}

	vtkLandmarkTransformPtr landmarktransform = vtkLandmarkTransformPtr::New();
	landmarktransform->SetSourceLandmarks(source);
	landmarktransform->SetTargetLandmarks(target);
	landmarktransform->SetModeToRigidBody();
	source->Modified();
	target->Modified();
	landmarktransform->Update();

	Transform3D tar_M_src(landmarktransform->GetMatrix());

	if (QString::number(tar_M_src(0,0))=="nan") // harry but quick way to check badness of transform...
	{
		return Transform3D::Identity();
	}

	*ok = true;
	return tar_M_src;
}

void RegistrationImplService::applyImage2ImageRegistration(Transform3D delta_pre_rMd, QString description)
{
	RegistrationTransform regTrans(delta_pre_rMd, QDateTime::currentDateTime(), description);
	regTrans.mFixed = mFixedData ? mFixedData->getUid() : "";
	regTrans.mMoving = mMovingData ? mMovingData->getUid() : "";

	this->updateRegistration(mLastRegistrationTime, regTrans, mMovingData);

	mLastRegistrationTime = regTrans.mTimestamp;
	reportSuccess(QString("Image registration [%1] has been performed on %2").arg(description).arg(regTrans.mMoving) );
}

//PatientModelService* RegistrationImplService::getPatientModelService()
//{
//	ctkServiceTracker<PatientModelService*> tracker(mContext);
//	tracker.open();
//	PatientModelService* service = tracker.getService(); // get arbitrary instance of this type
//	if(!service)
//		reportError("RegistrationImplService can't access PatientModelService");
//	return service;
//}

void RegistrationImplService::applyPatientRegistration(Transform3D rMpr_new, QString description)
{
	RegistrationTransform regTrans(rMpr_new, QDateTime::currentDateTime(), description);
	regTrans.mFixed = mFixedData ? mFixedData->getUid() : "";

	mPatientModelService->updateRegistration_rMpr(mLastRegistrationTime, regTrans);

	mLastRegistrationTime = regTrans.mTimestamp;
	reportSuccess(QString("Patient registration [%1] has been performed.").arg(description));
}

/** Update the registration for data and all data connected to its space.
 *
 * Registration is done relative to masterFrame, i.e. data is moved relative to the masterFrame.
 *
 */
void RegistrationImplService::updateRegistration(QDateTime oldTime, RegistrationTransform deltaTransform, DataPtr data)
{
	RegistrationApplicator applicator(mPatientModelService->getData());
	applicator.updateRegistration(oldTime, deltaTransform, data);
	mPatientModelService->autoSave();
}

/**\brief Identical to doFastRegistration_Orientation(), except data does not move.
 *
 * When applying a new transform to the patient orientation, all data is moved
 * the the inverse of that value, thus giving a net zero change along the path
 * pr...d_i.
 *
 */
void RegistrationImplService::applyPatientOrientation(const Transform3D& tMtm, const Transform3D& prMt)
{
	Transform3D rMpr = mPatientModelService->get_rMpr();

	//create a marked(m) space tm, which is related to tool space (t) as follows:
	//the tool is defined in DICOM space such that
	//the tool points toward the patients feet and the spheres faces the same
	//direction as the nose
	Transform3D tMpr = prMt.inv();

	// this is the new patient registration:
	Transform3D tmMpr = tMtm * tMpr;
	// the change in pat reg becomes:
	Transform3D F = tmMpr * rMpr.inv();

	QString description("Patient Orientation");

	QDateTime oldTime = this->getLastRegistrationTime(); // time of previous reg
	this->applyPatientRegistration(tmMpr, description);

	// now apply the inverse of F to all data,
	// thus ensuring the total path from pr to d_i is unchanged:
	Transform3D delta_pre_rMd = F;


	// use the same registration time as generated in the applyPatientRegistration() above:
	RegistrationTransform regTrans(delta_pre_rMd, this->getLastRegistrationTime(), description);

	std::map<QString,DataPtr> data = mPatientModelService->getData();
	// update the transform on all target data:
	for (std::map<QString,DataPtr>::iterator iter = data.begin(); iter!=data.end(); ++iter)
	{
		DataPtr current = iter->second;
		RegistrationTransform newTransform = regTrans;
		newTransform.mValue = regTrans.mValue * current->get_rMd();
		current->get_rMd_History()->updateRegistration(oldTime, newTransform);

		report("Updated registration of data " + current->getName());
		std::cout << "rMd_new\n" << newTransform.mValue << std::endl;
	}

	this->setLastRegistrationTime(regTrans.mTimestamp);

	reportSuccess("Patient Orientation has been performed");
}

void RegistrationImplService::writePreLandmarkRegistration(QString name, LandmarkMap landmarks)
{
	QStringList lm;
	for (LandmarkMap::iterator iter=landmarks.begin(); iter!=landmarks.end(); ++iter)
	{
		lm << mPatientModelService->getLandmarkProperties()[iter->second.getUid()].getName();
	}

	QString msg = QString("Preparing to register [%1] containing the landmarks: [%2]").arg(name).arg(lm.join(","));
	report(msg);
}

/**Convert the landmarks given by uids to vtk points.
 * The coordinates are given by the input data,
 * and should be transformed by M.
 *
 * Prerequisite: all uids exist in data.
 */
vtkPointsPtr RegistrationImplService::convertTovtkPoints(const std::vector<QString>& uids, const LandmarkMap& data, Transform3D M)
{
	vtkPointsPtr retval = vtkPointsPtr::New();

	for (unsigned i=0; i<uids.size(); ++i)
	{
		QString uid = uids[i];
		Vector3D p = M.coord(data.find(uid)->second.getCoord());
		retval->InsertNextPoint(p.begin());
	}
	return retval;
}

bool RegistrationImplService::isNull()
{
	return false;
}

} /* namespace cx */
