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


#ifndef CXDATAMANAGER_H_
#define CXDATAMANAGER_H_

#include "org_custusx_core_patientmodel_Export.h"
#include "cxPrecompiledHeader.h"

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <QObject>

#include "vtkForwardDeclarations.h"
#include "cxVector3D.h"
#include "cxLandmark.h"
#include "cxDefinitions.h"
#include "cxForwardDeclarations.h"
#include "cxTransform3D.h"

namespace cx
{
// forward declarations
typedef boost::shared_ptr<class TransferFunctions3DPresets> PresetTransferFunctions3DPtr;
typedef boost::shared_ptr<class RegistrationHistory> RegistrationHistoryPtr;
typedef boost::shared_ptr<class SpaceProvider> SpaceProviderPtr;
typedef boost::shared_ptr<class DataFactory> DataFactoryPtr;

/** Interface for a manager of data objects.
 *
 * Simply calling instance() will instantiate the default manager DataManagerImpl.
 * It is also possible to subclass and use setInstance() to set another type.
 *
 * \ingroup org_custusx_core_patientmodel
 */
class org_custusx_core_patientmodel_EXPORT DataManager: public QObject
{
Q_OBJECT
public:
	typedef std::map<QString, DataPtr> DataMap;
	typedef std::map<QString, ImagePtr> ImagesMap;
	typedef std::map<QString, MeshPtr> MeshMap;
	typedef std::map<QString, VideoSourcePtr> StreamMap;

	// streams
	virtual VideoSourcePtr getStream(const QString& uid) const = 0;
	virtual StreamMap getStreams() const = 0;
	virtual void loadStream(VideoSourcePtr stream) = 0;

	// images
	virtual std::map<QString, ImagePtr> getImages() const = 0;

	// meshes
	virtual std::map<QString, MeshPtr> getMeshes() const = 0;

	// data
	virtual void loadData(DataPtr data) = 0;
	virtual DataPtr loadData(const QString& uid, const QString& path) = 0;
    virtual std::map<QString, DataPtr> getData() const = 0;
	virtual DataPtr getData(const QString& uid) const = 0;
	virtual SpaceProviderPtr getSpaceProvider() = 0;
	virtual DataFactoryPtr getDataFactory() = 0;

	// global data (move to separate class if list grows)
	virtual Vector3D getCenter() const = 0; ///< current common center point for user viewing.
	virtual void setCenter(const Vector3D& center) = 0;

	virtual PresetTransferFunctions3DPtr getPresetTransferFunctions3D() const;

	virtual QString addLandmark() = 0;
	virtual void setLandmarkNames(std::vector<QString> names) = 0;
	virtual void setLandmarkName(QString uid, QString name) = 0;
	virtual void setLandmarkActive(QString uid, bool active) = 0;
	virtual LandmarkPropertyMap getLandmarkProperties() const = 0;
	virtual CLINICAL_VIEW getClinicalApplication() const = 0;
	virtual void setClinicalApplication(CLINICAL_VIEW application) = 0;
	virtual void clear() = 0; ///< remove all stuff from manager
	virtual void removeData(const QString& uid, QString basePath) = 0; ///< remove data from datamanger, emit signal

	virtual void addXml(QDomNode& parentNode) = 0; ///< adds xml information about the datamanger and its variabels
	virtual void parseXml(QDomNode& datamangerNode, QString absolutePath = QString()) = 0; ///< Use a XML node to load data. \param datamangerNode A XML data representation of the DataManager. \param absolutePath Absolute path to the data elements. Used together with the relative paths stored in the filePath elements.

	virtual Transform3D get_rMpr() const = 0; ///< get the patient registration transform
	virtual void set_rMpr(const Transform3D& val) = 0; ///<  set the transform from patient to reference space
	virtual RegistrationHistoryPtr get_rMpr_History() const = 0;
	virtual LandmarksPtr getPatientLandmarks() = 0;
	virtual void generateUidAndName(QString* _uid, QString* _name) = 0;

	//
	virtual ImagePtr getImage(const QString& uid) const;
	virtual MeshPtr getMesh(const QString& uid) const;
	virtual TrackedStreamPtr getTrackedStream(const QString& uid) const;

signals:
	void centerChanged(); ///< emitted when center is changed.
	void dataAddedOrRemoved();
	void landmarkPropertiesChanged(); ///< emitted when global info about a landmark changed
	void clinicalApplicationChanged();
	void streamLoaded();
	void rMprChanged(); ///< emitted when the transformation between patient reference and (data) reference is set

protected:
	DataManager();
	virtual ~DataManager();
};

} // namespace cx

#endif /*CXDATAMANAGER_H_*/
