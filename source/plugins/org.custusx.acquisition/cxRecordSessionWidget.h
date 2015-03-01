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

#ifndef CXRECORDSESSIONWIDGET_H_
#define CXRECORDSESSIONWIDGET_H_

#include "org_custusx_acquisition_Export.h"

#include "cxBaseWidget.h"

class QPushButton;
class QLineEdit;
class QLabel;

namespace cx
{
/**
* \file
* \addtogroup org_custusx_acquisition
* @{
*/

typedef boost::shared_ptr<class AcquisitionService> AcquisitionServicePtr;
typedef boost::shared_ptr<class RecordSessionWidget> RecordSessionWidgetPtr;

/**
 * \class RecordSessionWidget
 *
 * \brief
 *
 * \date Dec 8, 2010
 * \author Janne Beate Bakeng
 */
class org_custusx_acquisition_EXPORT  RecordSessionWidget : public BaseWidget
{
  Q_OBJECT

public:
  RecordSessionWidget(AcquisitionServicePtr base, QWidget* parent, QString defaultDescription = "Record Session", bool requireUsReady = true);
  virtual ~RecordSessionWidget();

  virtual QString defaultWhatsThis() const;

  void setDescription(QString text);
  void setDescriptionVisibility(bool value);

public slots:
	void setReady(bool val, QString text); ///< deprecated: use readinessChangedSlot instead.

private slots:
  void startStopSlot(bool);
  void cancelSlot();
  void recordStateChangedSlot();
  void usReadinessChangedSlot();

private:

  AcquisitionServicePtr mAcquisitionService;
  QLabel* mInfoLabel;
  QPushButton* mStartStopButton;
  QPushButton* mCancelButton;
  QLabel* mDescriptionLabel;
  QLineEdit* mDescriptionLine;
  bool mRequireUsReady;
};

/**
* @}
*/
}//namespace cx
#endif /* CXRECORDSESSIONWIDGET_H_ */
