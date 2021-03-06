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

#ifndef CXPROBECONFIGWIDGET_H_
#define CXPROBECONFIGWIDGET_H_

#include "cxResourceWidgetsExport.h"

#include "cxBaseWidget.h"
#include "cxBoundingBoxWidget.h"
#include "cxVector3DProperty.h"
#include "cxDoubleProperty.h"

namespace cx
{
typedef boost::shared_ptr<class VisServices> VisServicesPtr;
typedef boost::shared_ptr<class StringPropertyActiveProbeConfiguration> StringPropertyActiveProbeConfigurationPtr;

/**
 *
 * \brief Widget that displays/edits a probe configuration
 * \ingroup cx_gui
 *
 * \date Mar 16, 2012
 * \author Christian Askeland, SINTEF
 */
class cxResourceWidgets_EXPORT ProbeConfigWidget : public BaseWidget
{
	Q_OBJECT
public:
	ProbeConfigWidget(VisServicesPtr services, QWidget* parent=NULL);
	virtual ~ProbeConfigWidget();
	QWidget* getActiveProbeConfigWidget() { return mActiveProbeConfigWidget; }

private slots:
	void activeProbeConfigurationChangedSlot();
	void savePresetSlot();
	void deletePresetSlot();
	void guiImageSettingsChanged();
	void guiProbeSectorChanged();
	void guiOriginSettingsChanged();
	void syncBoxToSectorChanged();

private:
	VisServicesPtr mServices;
	QWidget* mActiveProbeConfigWidget;
	BoundingBoxWidget* mBBWidget;
	Vector3DPropertyBasePtr mOrigin;
	SliderRangeGroupWidget* mDepthWidget;
	DoublePropertyPtr mWidth;
	QCheckBox* mSyncBoxToSector;

	StringPropertyActiveProbeConfigurationPtr mActiveProbeConfig;
	QString mLastKnownProbeConfigName; ///< used for suggesting save names for new config
	bool mUpdating;
};

}

#endif /* CXPROBECONFIGWIDGET_H_ */
