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

#ifndef CXCLIPPERWIDGET_H
#define CXCLIPPERWIDGET_H

#include "cxGuiExport.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include "cxBaseWidget.h"
#include "cxStringPropertyBase.h"
#include "cxForwardDeclarations.h"
#include "cxData.h"
class QTableWidget;

namespace cx
{
typedef boost::shared_ptr<class StringPropertySelectTool> StringPropertySelectToolPtr;
typedef boost::shared_ptr<class StringPropertyClipPlane> StringPropertyClipPlanePtr;

class LabeledComboBoxWidget;

class cxGui_EXPORT ClipperWidget : public BaseWidget
{
	Q_OBJECT
	void updateCheckBoxFromClipper(QCheckBox *checkbox, DataPtr data);
public:
	ClipperWidget(VisServicesPtr services, QWidget *parent);
	~ClipperWidget();
	void setClipper(InteractiveClipperPtr clipper);
protected slots:
	void setupDataSelectorUI();
	void enable(bool checked);
	void dataTypeSelectorClicked(bool checked);
	void selectAllTableData(bool checked);
	void dataSelectorClicked(QCheckBox *checkBox, DataPtr data);
	void onToolChanged();
protected:
	void setupUI();
	virtual void prePaintEvent();

	InteractiveClipperPtr mClipper;
//	StringPropertyClipPlanePtr mPlaneAdapter;
//	LabeledComboBoxWidget* planeSelector;
	QVBoxLayout* mLayout;
	QCheckBox* mUseClipperCheckBox;
	QCheckBox *mAttachedToTool;
	QCheckBox *mSelectAllData;
	QCheckBox *mInvertPlane;

	QCheckBox *mShowImages;
	QCheckBox *mShowMeshes;
	QCheckBox *mShowMetrics;
	QCheckBox *mShowTrackedStreams;

	VisServicesPtr mServices;
	std::map<QString, DataPtr> mDataToClip;
	QTableWidget *mDataTableWidget;
	std::map<QString, DataPtr> getDatas();
	bool mInitializedWithClipper;
	StringPropertySelectToolPtr mToolSelector;
	void setupDataStructures();
	void connectToNewClipper();
	QGroupBox *dataTableWidget();
	QLayout *planeLayout();
	QLayout *toolLayout();
	QString getDataTypeRegExp();
	void updateSelectAllCheckbox();
	void createNewCheckboxesBasedOnData();
};

}

#endif // CXCLIPPERWIDGET_H
