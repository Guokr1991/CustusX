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

#include "cxTransferFunctionWidget.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QStringList>
#include <QInputDialog>
#include <QPushButton>
#include <QActionGroup>
#include <QToolButton>
#include <QAction>
#include <QMessageBox>

#include "cxImageTF3D.h"
#include "cxImageLUT2D.h"

#include "cxTypeConversions.h"
#include "cxTransferFunctionPresetWidget.h"
#include "cxTransferFunctionAlphaWidget.h"
#include "cxTransferFunctionColorWidget.h"
#include "cxShadingWidget.h"
#include "cxDataViewSelectionWidget.h"
#include "cxShadingParamsInterfaces.h"
#include "cxSettings.h"
#include "cxPatientModelService.h"
#include "cxLogicManager.h"

namespace cx
{

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

DoublePropertyImageTFDataBase::DoublePropertyImageTFDataBase()
{
}

void DoublePropertyImageTFDataBase::setImageTFData(ImageTFDataPtr tfData, ImagePtr image)
{
  if (mImageTFData)
	disconnect(mImageTFData.get(), &ImageTFData::transferFunctionsChanged, this, &Property::changed);

  mImageTFData = tfData;
  mImage = image;

  if (mImageTFData)
	connect(mImageTFData.get(), &ImageTFData::transferFunctionsChanged, this, &Property::changed);

  emit changed();
}

double DoublePropertyImageTFDataBase::getValue() const
{
  if (!mImageTFData)
    return 0.0;
  return this->getValueInternal();
}

bool DoublePropertyImageTFDataBase::setValue(double val)
{
  if (!mImageTFData)
    return false;
  this->setValueInternal(val);
  return true;
}

//---------------------------------------------------------
//---------------------------------------------------------

double DoublePropertyImageTFDataWindow::getValueInternal() const
{
  return mImageTFData->getWindow();
}

void DoublePropertyImageTFDataWindow::setValueInternal(double val)
{
  mImageTFData->setWindow(val);
}

DoubleRange DoublePropertyImageTFDataWindow::getValueRange() const
{
  if (!mImage)
    return DoubleRange();
  double range = mImage->getMax() - mImage->getMin();
  return DoubleRange(1,range,range/1000.0);
}

//---------------------------------------------------------
//---------------------------------------------------------

double DoublePropertyImageTFDataLevel::getValueInternal() const
{
  return mImageTFData->getLevel();
}

void DoublePropertyImageTFDataLevel::setValueInternal(double val)
{
  mImageTFData->setLevel(val);
}

DoubleRange DoublePropertyImageTFDataLevel::getValueRange() const
{
  if (!mImageTFData)
    return DoubleRange();

  double max = mImage->getMax();
  double min = mImage->getMin();
  return DoubleRange(min,max,1);
}

//---------------------------------------------------------
//---------------------------------------------------------

double DoublePropertyImageTFDataLLR::getValueInternal() const
{
  return mImageTFData->getLLR();
}
void DoublePropertyImageTFDataLLR::setValueInternal(double val)
{
  mImageTFData->setLLR(val);
}
DoubleRange DoublePropertyImageTFDataLLR::getValueRange() const
{
  if (!mImageTFData)
    return DoubleRange();

  double max = mImage->getMax();
  double min = mImage->getMin();
	//Set range to min - 1 to allow an llr that shows all values
	return DoubleRange(min - 1,max,(max-min)/1000.0);
}

//---------------------------------------------------------
//---------------------------------------------------------

double DoublePropertyImageTFDataAlpha::getValueInternal() const
{
  return mImageTFData->getAlpha();
}
void DoublePropertyImageTFDataAlpha::setValueInternal(double val)
{
  mImageTFData->setAlpha(val);
}
DoubleRange DoublePropertyImageTFDataAlpha::getValueRange() const
{
  if (!mImageTFData)
    return DoubleRange();

  double max = 1.0;
  return DoubleRange(0,max,max/100.0);
}

//---------------------------------------------------------
//---------------------------------------------------------

TransferFunction3DWidget::TransferFunction3DWidget(PatientModelServicePtr patientModelService, QWidget* parent, bool connectToActiveImage) :
  BaseWidget(parent, "TransferFunction3DWidget", "3D"),
  mLayout(new QVBoxLayout(this)),
  mPatientModelService(patientModelService),
  mActiveImageProxy(ActiveImageProxyPtr())
{
  mTransferFunctionAlphaWidget = new TransferFunctionAlphaWidget(patientModelService, this);
  mTransferFunctionColorWidget = new TransferFunctionColorWidget(patientModelService, this);

  mTransferFunctionAlphaWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                                              QSizePolicy::MinimumExpanding);
  mTransferFunctionColorWidget->setSizePolicy(QSizePolicy::Expanding,
                                              QSizePolicy::Fixed);

  mLayout->addWidget(mTransferFunctionAlphaWidget);
  mLayout->addWidget(mTransferFunctionColorWidget);

  this->setLayout(mLayout);

  if(connectToActiveImage)
  {
	  mActiveImageProxy = ActiveImageProxy::New(patientModelService);
	  connect(mActiveImageProxy.get(), &ActiveImageProxy::activeImageChanged, this, &TransferFunction3DWidget::activeImageChangedSlot);
	  connect(mActiveImageProxy.get(), &ActiveImageProxy::transferFunctionsChanged, this, &TransferFunction3DWidget::activeImageChangedSlot);
  }
}

QString TransferFunction3DWidget::defaultWhatsThis() const
{
  return "<html>"
    "<h3>3D Transfer Function</h3>"
    "<p>Lets you set a transfer function on a 3D volume.</p>"
    "<p><i></i></p>"
    "</html>";
}

void TransferFunction3DWidget::activeImageChangedSlot()
{
  ImagePtr activeImage = mPatientModelService->getActiveImage();
  this->imageChangedSlot(activeImage);
}

void TransferFunction3DWidget::imageChangedSlot(ImagePtr image)
{
	ImageTFDataPtr tf;
	if (image)
	  tf = image->getTransferFunctions3D();
	else
	  image.reset();

	mTransferFunctionAlphaWidget->setData(image, tf);
	mTransferFunctionColorWidget->setData(image, tf);
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

TransferFunction2DWidget::TransferFunction2DWidget(PatientModelServicePtr patientModelService, QWidget* parent, bool connectToActiveImage) :
  BaseWidget(parent, "TransferFunction2DWidget", "2D"),
  mLayout(new QVBoxLayout(this)),
  mPatientModelService(patientModelService)
{
  mTransferFunctionAlphaWidget = new TransferFunctionAlphaWidget(patientModelService, this);
  mTransferFunctionAlphaWidget->setReadOnly(true);
  mTransferFunctionColorWidget = new TransferFunctionColorWidget(patientModelService, this);

  mDataWindow.reset(new DoublePropertyImageTFDataWindow);
  mDataLevel.reset(new DoublePropertyImageTFDataLevel);
  mDataAlpha.reset(new DoublePropertyImageTFDataAlpha);
  mDataLLR.reset(new DoublePropertyImageTFDataLLR);

  mActiveImageProxy = ActiveImageProxy::New(patientModelService);
  connect(mActiveImageProxy.get(), &ActiveImageProxy::activeImageChanged, this, &TransferFunction2DWidget::activeImageChangedSlot);
  connect(mActiveImageProxy.get(), &ActiveImageProxy::transferFunctionsChanged, this, &TransferFunction2DWidget::activeImageChangedSlot);

  mTransferFunctionAlphaWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                                              QSizePolicy::MinimumExpanding);
  mTransferFunctionColorWidget->setSizePolicy(QSizePolicy::Expanding,
                                              QSizePolicy::Fixed);

  mLayout->addWidget(mTransferFunctionAlphaWidget);
  mLayout->addWidget(mTransferFunctionColorWidget);

  QGridLayout* gridLayout = new QGridLayout;
  mLayout->addLayout(gridLayout);
  new SliderGroupWidget(this, mDataWindow, gridLayout, 0);
  new SliderGroupWidget(this, mDataLevel,  gridLayout, 1);
  new SliderGroupWidget(this, mDataAlpha,  gridLayout, 2);
  new SliderGroupWidget(this, mDataLLR,    gridLayout, 3);

  this->setLayout(mLayout);
}

QString TransferFunction2DWidget::defaultWhatsThis() const
{
  return "<html>"
    "<h3>2D Transfer Function</h3>"
    "<p>Lets you set a transfer function on a 2D image.</p>"
    "<p><i></i></p>"
    "</html>";
}

void TransferFunction2DWidget::activeImageChangedSlot()
{
  ImagePtr image = mPatientModelService->getActiveImage();
  ImageTFDataPtr tf;
  if (image)
    tf = image->getLookupTable2D();
  else
    image.reset();

  mTransferFunctionAlphaWidget->setData(image, tf);
  mTransferFunctionColorWidget->setData(image, tf);

  mDataWindow->setImageTFData(tf, image);
  mDataLevel->setImageTFData(tf, image);
  mDataAlpha->setImageTFData(tf, image);
  mDataLLR->setImageTFData(tf, image);
}


//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

TransferFunctionWidget::TransferFunctionWidget(PatientModelServicePtr patientModelService, QWidget* parent, bool connectToActiveImage) :
  BaseWidget(parent, "TransferFunctionWidget", "Transfer Function")
{
  QVBoxLayout* mLayout = new QVBoxLayout(this);

  TransferFunction3DWidget* transferFunctionWidget = new TransferFunction3DWidget(patientModelService, this, connectToActiveImage);

  mLayout->setMargin(0);
  mLayout->addWidget(transferFunctionWidget);
  mLayout->addWidget(new TransferFunctionPresetWidget(patientModelService, this, true));

  this->setLayout(mLayout);
}

QString TransferFunctionWidget::defaultWhatsThis() const
{
  return "<html>"
    "<h3>Transfer Function.</h3>"
    "<p>Lets you set a new or predefined transfer function on a volume.</p>"
    "<p><i></i></p>"
    "</html>";
}
}//namespace cx
