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
#include "cxMeshInfoWidget.h"

#include <QVBoxLayout>
#include "cxImage.h"

#include "cxRegistrationTransform.h"
#include "cxLabeledComboBoxWidget.h"
#include "cxLabeledLineEditWidget.h"
#include "cxMeshHelpers.h"
#include "cxHelperWidgets.h"
#include "cxColorDataAdapterXml.h"
#include "cxDataLocations.h"
#include "cxDataInterface.h"
#include "cxDataSelectWidget.h"
#include "cxSelectDataStringProperty.h"

#include "cxPatientModelService.h"
#include "cxLogger.h"

namespace cx
{


MeshInfoWidget::MeshInfoWidget(PatientModelServicePtr patientModelService, VisualizationServicePtr visualizationService, QWidget* parent) :
	InfoWidget(parent, "MeshInfoWidget", "Mesh Properties"),
	mPatientModelService(patientModelService),
	mVisualizationService(visualizationService)
{
	this->addWidgets(patientModelService);
	this->meshSelectedSlot();
}

MeshInfoWidget::~MeshInfoWidget()
{}

void MeshInfoWidget::setColorSlot()
{
  if(!mMesh)
    return;
  // Implement like TransferFunctionColorWidget::setColorSlot()
  // to prevent crash problems
  QTimer::singleShot(1, this, SLOT(setColorSlotDelayed()));
}

void MeshInfoWidget::setColorSlotDelayed()
{
  mMesh->setColor(mColorAdapter->getValue());
}

void MeshInfoWidget::meshSelectedSlot()
{
	if (mMesh == mSelectMeshWidget->getMesh())
	return;

	if(mMesh)
	{
		disconnect(mBackfaceCullingCheckBox, SIGNAL(toggled(bool)), mMesh.get(), SLOT(setBackfaceCullingSlot(bool)));
		disconnect(mFrontfaceCullingCheckBox, SIGNAL(toggled(bool)), mMesh.get(), SLOT(setFrontfaceCullingSlot(bool)));
		disconnect(mMesh.get(), SIGNAL(meshChanged()), this, SLOT(meshChangedSlot()));
	}

	mMesh = mSelectMeshWidget->getMesh();

	if (!mMesh)
	{
		mParentFrameAdapter->setData(mMesh);
		mNameAdapter->setData(mMesh);
		mUidAdapter->setData(mMesh);
		return;
	}

	mBackfaceCullingCheckBox->setChecked(mMesh->getBackfaceCulling());
	mFrontfaceCullingCheckBox->setChecked(mMesh->getFrontfaceCulling());
	connect(mBackfaceCullingCheckBox, SIGNAL(toggled(bool)), mMesh.get(), SLOT(setBackfaceCullingSlot(bool)));
	connect(mFrontfaceCullingCheckBox, SIGNAL(toggled(bool)), mMesh.get(), SLOT(setFrontfaceCullingSlot(bool)));
	connect(mMesh.get(), SIGNAL(meshChanged()), this, SLOT(meshChangedSlot()));

	mParentFrameAdapter->setData(mMesh);
	mNameAdapter->setData(mMesh);
	mUidAdapter->setData(mMesh);
	mColorAdapter->setValue(mMesh->getColor());

	std::map<std::string, std::string> info = getDisplayFriendlyInfo(mMesh);
	this->populateTableWidget(info);
}

void MeshInfoWidget::importTransformSlot()
{
  if(!mMesh)
    return;
  DataPtr parent = mPatientModelService->getData(mMesh->getParentSpace());
  if (!parent)
    return;
  mMesh->get_rMd_History()->setRegistration(parent->get_rMd());
  report("Assigned rMd from volume [" + parent->getName() + "] to surface [" + mMesh->getName() + "]");
}
  
void MeshInfoWidget::meshChangedSlot()
{
	mBackfaceCullingCheckBox->setChecked(mMesh->getBackfaceCulling());
	mFrontfaceCullingCheckBox->setChecked(mMesh->getFrontfaceCulling());
	mColorAdapter->setValue(mMesh->getColor());
}

void MeshInfoWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
}

void MeshInfoWidget::hideEvent(QCloseEvent* event)
{
  QWidget::closeEvent(event);
}

void MeshInfoWidget::addWidgets(PatientModelServicePtr patientModelService)
{
	mSelectMeshWidget = SelectMeshStringDataAdapter::New(patientModelService);
	mSelectMeshWidget->setValueName("Surface: ");
	connect(mSelectMeshWidget.get(), SIGNAL(changed()), this, SLOT(meshSelectedSlot()));

	XmlOptionFile options = XmlOptionFile(DataLocations::getXmlSettingsFile()).descend("MeshInfoWidget");
	QString uid("Color");
	QString name("");
	QString help("Color of the mesh.");
	QColor color("red");

	if(mSelectMeshWidget->getMesh())
		color = mSelectMeshWidget->getMesh()->getColor();

	mColorAdapter = ColorDataAdapterXml::initialize(uid, name, help, color, options.getElement());
	connect(mColorAdapter.get(), SIGNAL(changed()), this, SLOT(setColorSlot()));

	QPushButton* importTransformButton = new QPushButton("Import Transform from Parent", this);
	importTransformButton->setToolTip("Replace data transform with that of the parent data.");
	connect(importTransformButton, SIGNAL(clicked()), this, SLOT(importTransformSlot()));

	mUidAdapter = DataUidEditableStringDataAdapter::New();
	mNameAdapter = DataNameEditableStringDataAdapter::New();
	mParentFrameAdapter = ParentFrameStringDataAdapter::New(mPatientModelService);

	QWidget* optionsWidget = new QWidget(this);
	QHBoxLayout* optionsLayout = new QHBoxLayout(optionsWidget);
	mBackfaceCullingCheckBox = new QCheckBox("Backface culling");
	mBackfaceCullingCheckBox->setToolTip("Set backface culling on. This makes transparent meshes work, but only draws outside mesh walls (eg. navigating inside meshes will not work).");
	optionsLayout->addWidget(mBackfaceCullingCheckBox);
	mFrontfaceCullingCheckBox = new QCheckBox("Frontface culling");
	mFrontfaceCullingCheckBox->setToolTip("Set frontface culling on. Can be used to make transparent meshes work from inside the meshes.");
	optionsLayout->addWidget(mFrontfaceCullingCheckBox);
	optionsLayout->addWidget(sscCreateDataWidget(this, mColorAdapter));
	optionsLayout->addStretch(1);

	int gridLayoutRow = 1;

	gridLayout->addWidget(new DataSelectWidget(mVisualizationService, mPatientModelService, this, mSelectMeshWidget), gridLayoutRow++, 0, 1, 2);
	new LabeledLineEditWidget(this, mUidAdapter, gridLayout, gridLayoutRow++);
	new LabeledLineEditWidget(this, mNameAdapter, gridLayout, gridLayoutRow++);
	new LabeledComboBoxWidget(this, mParentFrameAdapter, gridLayout, gridLayoutRow++);
	gridLayout->addWidget(optionsWidget, gridLayoutRow++, 0, 1, 2);
	gridLayout->addWidget(mTableWidget, gridLayoutRow++, 0, 1, 2);
	gridLayout->addWidget(importTransformButton, gridLayoutRow++, 0, 1, 2);

	this->addStretch();
}

}//end namespace cx
