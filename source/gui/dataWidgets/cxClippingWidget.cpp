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
#include "cxClippingWidget.h"

#include <ctkPluginContext.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include "cxStringDataAdapter.h"
#include "cxLabeledComboBoxWidget.h"
#include "cxDefinitionStrings.h"
#include "cxInteractiveClipper.h"
#include "cxViewManager.h"
#include "cxDataManager.h"
#include "cxSelectDataStringDataAdapter.h"
#include "cxImage.h"
#include "cxLegacySingletons.h"
namespace cx
{

ClipPlaneStringDataAdapter::ClipPlaneStringDataAdapter(InteractiveClipperPtr clipper) :
	mInteractiveClipper(clipper)
{
	connect(mInteractiveClipper.get(), SIGNAL(changed()), this, SIGNAL(changed()));
}

QString ClipPlaneStringDataAdapter::getDisplayName() const
{
	return "Slice Plane";
}
bool ClipPlaneStringDataAdapter::setValue(const QString& value)
{
	PLANE_TYPE plane = string2enum<PLANE_TYPE> (value);
	if (plane == mInteractiveClipper->getSlicePlane())
		return false;
	mInteractiveClipper->setSlicePlane(plane);
	return true;
}
QString ClipPlaneStringDataAdapter::getValue() const
{
	return qstring_cast(mInteractiveClipper->getSlicePlane());
}
QString ClipPlaneStringDataAdapter::getHelp() const
{
	return "Chose the slice plane to clip with.";
}
QStringList ClipPlaneStringDataAdapter::getValueRange() const
{
	std::vector<PLANE_TYPE> planes = mInteractiveClipper->getAvailableSlicePlanes();
	QStringList retval;
	//retval << ""; // removed this. No idea why we need an empty entry.
	for (unsigned i = 0; i < planes.size(); ++i)
		retval << qstring_cast(planes[i]);
	return retval;
}

///--------------------------------------------------------
///--------------------------------------------------------
///--------------------------------------------------------

ClippingWidget::ClippingWidget(ctkPluginContext *pluginContext, QWidget* parent) :
	BaseWidget(parent, "ClippingWidget", "Clip")
{
	mInteractiveClipper = viewManager()->getClipper();
	connect(mInteractiveClipper.get(), SIGNAL(changed()), this, SLOT(clipperChangedSlot()));

	mImageAdapter = SelectImageStringDataAdapter::New(pluginContext);
	LabeledComboBoxWidget* imageCombo = new LabeledComboBoxWidget(this, mImageAdapter);
	connect(mImageAdapter.get(), SIGNAL(changed()), this, SLOT(imageChangedSlot()));

	this->setToolTip(this->defaultWhatsThis());

	QVBoxLayout* layout = new QVBoxLayout(this);

	QGroupBox* activeClipGroupBox = new QGroupBox("Interactive clipper");
	activeClipGroupBox->setToolTip(this->defaultWhatsThis());
	layout->addWidget(activeClipGroupBox);
	QVBoxLayout* activeClipLayout = new QVBoxLayout(activeClipGroupBox);

	mPlaneAdapter = ClipPlaneStringDataAdapter::New(mInteractiveClipper);
	LabeledComboBoxWidget* combo = new LabeledComboBoxWidget(this, mPlaneAdapter);

	mUseClipperCheckBox = new QCheckBox("Use Clipper");
	mUseClipperCheckBox->setToolTip("Turn on interactive clipping for the selected volume.");
	connect(mUseClipperCheckBox, SIGNAL(toggled(bool)), mInteractiveClipper.get(), SLOT(useClipper(bool)));
	activeClipLayout->addWidget(mUseClipperCheckBox);
	activeClipLayout->addWidget(imageCombo);
	activeClipLayout->addWidget(combo);
	mInvertPlaneCheckBox = new QCheckBox("Invert plane");
	mInvertPlaneCheckBox->setToolTip("Use the inverse (mirror) of the selected slice plane.");
	connect(mInvertPlaneCheckBox, SIGNAL(toggled(bool)), mInteractiveClipper.get(), SLOT(invertPlane(bool)));
	activeClipLayout->addWidget(mInvertPlaneCheckBox);

	QPushButton* saveButton = new QPushButton("Save clip plane");
	saveButton->setToolTip("Save the interactive plane as a clip plane in the selected volume.");
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveButtonClickedSlot()));
	//saveButton->setEnabled(false);
	QPushButton* clearButton = new QPushButton("Clear saved planes");
	clearButton->setToolTip("Remove all saved clip planes from the selected volume");
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clearButtonClickedSlot()));
	//clearButton->setEnabled(false);
	activeClipLayout->addWidget(saveButton);
	layout->addWidget(clearButton);

	layout->addStretch();

	this->clipperChangedSlot();
}

QString ClippingWidget::defaultWhatsThis() const
{
	return "<html>"
		"<h3>Functonality for clipping a volume</h3>"
		"<p>"
		"Define clip planes in a volume. The interactive clipper is attached "
		"to the active tool, and clips the active volume according to a slice "
		"definition. "
		"</p>"
		"<p>"
		"The current clip can also be saved along with the volume. This can be "
		"done several times."
		"</p>"
		"<p><i></i></p>"
		"</html>";
}

void ClippingWidget::clipperChangedSlot()
{
	mUseClipperCheckBox->setChecked(mInteractiveClipper->getUseClipper());
	mInvertPlaneCheckBox->setChecked(mInteractiveClipper->getInvertPlane());
	if (mInteractiveClipper->getImage())
		mImageAdapter->setValue(mInteractiveClipper->getImage()->getUid());
}

void ClippingWidget::imageChangedSlot()
{
	mInteractiveClipper->setImage(dataManager()->getImage(mImageAdapter->getValue()));
}

void ClippingWidget::clearButtonClickedSlot()
{
	mInteractiveClipper->clearClipPlanesInVolume();
}

void ClippingWidget::saveButtonClickedSlot()
{
	mInteractiveClipper->saveClipPlaneToVolume();
}

}
