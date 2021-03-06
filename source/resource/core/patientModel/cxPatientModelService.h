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

#ifndef CXPATIENTMODELSERVICE_H_
#define CXPATIENTMODELSERVICE_H_

#include "cxResourceExport.h"

#include <QObject>
#include <map>
#include "boost/shared_ptr.hpp"
#include "cxTransform3D.h"
#include "cxForwardDeclarations.h"
#include "vtkForwardDeclarations.h"
#include "cxDefinitions.h"

class QDateTime;
class QDomElement;

namespace cx
{
typedef boost::shared_ptr<class Data> DataPtr;
typedef boost::shared_ptr<class Image> ImagePtr;
}

#define PatientModelService_iid "cx::PatientModelService"

namespace cx
{
class RegistrationTransform;
class LandmarkProperty;
typedef boost::shared_ptr<class PatientModelService> PatientModelServicePtr;
typedef boost::shared_ptr<class Landmarks> LandmarksPtr;
typedef boost::shared_ptr<class TransferFunctions3DPresets> PresetTransferFunctions3DPtr;
typedef boost::shared_ptr<class RegistrationHistory> RegistrationHistoryPtr;

/** \brief The virtual patient
 *
 * PatientModelService provides access to the Patient Specific Model (PaSM).
 *   - data entities
 *   	- volumes
 *   	- surfaces
 *   	- metrics
 *   	- etc
 *   - relations between entities in space, time and structure
 *   - load/save
 *
 *
 *  \ingroup cx_resource_core_data
 *  \date 2014-05-15
 *  \author Christian Askeland, SINTEF
 *  \author Ole Vegard Solberg, SINTEF
 */
class cxResource_EXPORT PatientModelService : public QObject
{
	Q_OBJECT
public:
	virtual ~PatientModelService() {}

	// core Data interface
	virtual void insertData(DataPtr data) = 0;
	virtual std::map<QString, DataPtr> getData() const = 0;
	/** Create Data object of given type.
	 *
	 *  uid must be unique, or contain the string %1 that will be replaced with a running
	 *  index that makes the uid unique. The same applies to name. An empty name will set
	 *  name = uid.
	 */
	virtual DataPtr createData(QString type, QString uid, QString name="") = 0;
	virtual void removeData(QString uid) = 0;

	// extended Data interface
	template <class DATA>
	std::map<QString, boost::shared_ptr<DATA> > getDataOfType() const;
	DataPtr getData(const QString& uid) const;
	template <class DATA>
	boost::shared_ptr<DATA> getData(const QString& uid) const;
	template<class DATA>
	boost::shared_ptr<DATA> createSpecificData(QString uid, QString name="");

	// streams
	virtual std::map<QString, VideoSourcePtr> getStreams() const = 0;
	VideoSourcePtr getStream(const QString &uid) const; ///< Convenience function getting a specified stream
	// patient registration
	virtual Transform3D get_rMpr() const;
	virtual RegistrationHistoryPtr get_rMpr_History() const = 0;

	// active data
	virtual ActiveDataPtr getActiveData() const = 0;

	// landmarks
	virtual LandmarksPtr getPatientLandmarks() const = 0; ///< landmark defined in patient space
	/** Get all defined landmarks.
	 *  These landmarks are additionally defined in specific coordinate spaces,
	 *  such as patient and for each Data.
	 */
	virtual std::map<QString, LandmarkProperty> getLandmarkProperties() const = 0;
	virtual void setLandmarkName(QString uid, QString name) = 0;
	virtual void setLandmarkActive(QString uid, bool active) = 0;
	virtual QString addLandmark() = 0;
	// utility
	virtual void updateRegistration_rMpr(const QDateTime& oldTime, const RegistrationTransform& newTransform, bool continuous);

	virtual QString getActivePatientFolder() const = 0;
	QString generateFilePath(QString folderName, QString ending);

	virtual bool isPatientValid() const = 0;
	virtual DataPtr importData(QString fileName, QString &infoText) = 0;
	virtual void exportPatient(PATIENT_COORDINATE_SYSTEM externalSpace) = 0;

	virtual PresetTransferFunctions3DPtr getPresetTransferFunctions3D() const = 0;

	virtual void setCenter(const Vector3D& center) = 0;
	virtual Vector3D getCenter() const = 0; ///< current common center point for user viewing.

	virtual CLINICAL_VIEW getClinicalApplication() const = 0;
	virtual void setClinicalApplication(CLINICAL_VIEW application) = 0;

	virtual void autoSave() = 0;//TODO remove, and integrate into other functions
	virtual bool isNull() = 0;

	static PatientModelServicePtr getNullObject();

signals:
	void centerChanged(); ///< emitted when center is changed.
	void dataAddedOrRemoved();
	void landmarkPropertiesChanged(); ///< emitted when global info about a landmark changed
	void clinicalApplicationChanged();
	void rMprChanged();
	void streamLoaded();
	void patientChanged();
	void videoAddedToTrackedStream();
};


template <class DATA>
std::map<QString, boost::shared_ptr<DATA> > PatientModelService::getDataOfType() const
{
	std::map<QString, DataPtr> data = this->getData();
	std::map<QString, boost::shared_ptr<DATA> > retval;
	for (std::map<QString, DataPtr>::const_iterator i=data.begin(); i!=data.end(); ++i)
	{
		boost::shared_ptr<DATA> val = boost::dynamic_pointer_cast<DATA>(i->second);
		if (val)
			retval[val->getUid()] = val;
	}
	return retval;
}

template <class DATA>
boost::shared_ptr<DATA> PatientModelService::getData(const QString& uid) const
{
	return boost::dynamic_pointer_cast<DATA>(this->getData(uid));
}

template<class DATA>
boost::shared_ptr<DATA> PatientModelService::createSpecificData(QString uid, QString name)
{
	DataPtr retval = this->createData(DATA::getTypeName(), uid, name);
	return boost::dynamic_pointer_cast<DATA>(retval);
}

} // namespace cx
Q_DECLARE_INTERFACE(cx::PatientModelService, PatientModelService_iid)


#endif /* CXPATIENTMODELSERVICE_H_ */
