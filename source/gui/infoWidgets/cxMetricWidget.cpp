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

/*
 * cxMetricWidget.cpp
 *
 *  \date Jul 5, 2011
 *      \author christiana
 */

#include <cxMetricWidget.h>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QVBoxLayout>
#include <QHeaderView>


#include "cxTypeConversions.h"
#include "cxCoordinateSystemHelpers.h"
#include "cxTrackingService.h"
#include "cxLabeledComboBoxWidget.h"
#include "cxVector3DWidget.h"
#include "cxTimeKeeper.h"
#include "cxFrameMetricWrapper.h"
#include "cxToolMetricWrapper.h"
#include "cxTime.h"
#include "cxMetricManager.h"

#include "cxPatientModelService.h"


namespace cx
{


//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

MetricWidget::MetricWidget(ViewServicePtr viewService, PatientModelServicePtr patientModelService, QWidget* parent) :
  BaseWidget(parent, "MetricWidget", "Metrics/3D ruler"),
  mVerticalLayout(new QVBoxLayout(this)),
  mTable(new QTableWidget(this)),
  mPatientModelService(patientModelService),
  mViewService(viewService)
{
	// the delayed timer lowers the update rate of this widget,
	// as is is seen to strangle the render speed when many metrics are present.
	int lowUpdateRate = 100;
	mLocalModified = false;
	mDelayedUpdateTimer = new QTimer(this);
	connect(mDelayedUpdateTimer, SIGNAL(timeout()), this, SLOT(delayedUpdate())); // this signal will be executed in the thread of THIS, i.e. the main thread.
	mDelayedUpdateTimer->start(lowUpdateRate);
	this->setToolTip("3D measurements");

	mModifiedCount = 0;
	mPaintCount = 0;
	mMetricManager.reset(new MetricManager());
	connect(mMetricManager.get(), SIGNAL(activeMetricChanged()), this, SLOT(setModified()));
	connect(mMetricManager.get(), SIGNAL(metricsChanged()), this, SLOT(setModified()));

  //table widget
  connect(mTable, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
  connect(mTable, SIGNAL(cellChanged(int, int)), this, SLOT(cellChangedSlot(int, int)));
  connect(mTable, SIGNAL(cellClicked(int, int)), this, SLOT(cellClickedSlot(int, int)));

  this->setLayout(mVerticalLayout);

  mEditWidgets = new QStackedWidget;

  QActionGroup* group = new QActionGroup(this);
  this->createActions(group);

//  QToolBar* toolBar = new QToolBar("actions", this);
//  toolBar->addActions(group->actions());

  QWidget* toolBar = new QWidget(this);
  QHBoxLayout* toolLayout = new QHBoxLayout(toolBar);
  toolLayout->setMargin(0);
  toolLayout->setSpacing(0);
  QList<QAction*> actions = group->actions();
  for (unsigned i=0; i<actions.size(); ++i)
  {
	  if (actions[i]->isSeparator())
	  {
		  toolLayout->addSpacing(4);
		  QFrame* frame = new QFrame();
		  frame->setFrameStyle(QFrame::Sunken + QFrame::VLine);
		  toolLayout->addWidget(frame);
		  toolLayout->addSpacing(4);
	  }
	  else
	  {
		  CXNoBorderToolButton* button = new CXNoBorderToolButton;
		  button->setDefaultAction(actions[i]);
		  button->setIconSize(QSize(32,32));
		  toolLayout->addWidget(button);
	  }
  }

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(toolBar);
  buttonLayout->addStretch();

  //layout
  mVerticalLayout->addLayout(buttonLayout);
  mVerticalLayout->addWidget(mTable, 1);
  mVerticalLayout->addWidget(mEditWidgets, 0);
}

MetricWidget::~MetricWidget()
{}

void MetricWidget::createActions(QActionGroup* group)
{
	mPointMetricAction = this->createAction(group, ":/icons/metric_point.png", "Pt", "Create a new Point Metric", SLOT(addPointButtonClickedSlot()));
	mFrameMetricAction = this->createAction(group, ":/icons/metric_frame.png", "Frame", "Create a new Frame Metric (position and orientation)", SLOT(addFrameButtonClickedSlot()));
	mToolMetricAction = this->createAction(group, ":/icons/metric_tool.png", "Tool", "Create a new Tool Metric", SLOT(addToolButtonClickedSlot()));
	this->createAction(group, ":/icons/metric_distance.png", "Dist", "Create a new Distance Metric", SLOT(addDistanceButtonClickedSlot()));
	this->createAction(group, ":/icons/metric_angle.png", "Angle", "Create a new Angle Metric",   SLOT(addAngleButtonClickedSlot()));
	this->createAction(group, ":/icons/metric_plane.png", "Plane", "Create a new Plane Metric",   SLOT(addPlaneButtonClickedSlot()));
	this->createAction(group, ":/icons/metric_sphere.png", "Sphere", "Create a new SphereMetric",   SLOT(addSphereButtonClickedSlot()));
	this->createAction(group, ":/icons/metric_torus.png", "Torus", "Create a new Torus Metric",   SLOT(addDonutButtonClickedSlot()));

	this->createAction(group, "", "", "", NULL)->setSeparator(true);
	mRemoveAction = this->createAction(group, ":/icons/metric_remove.png", "Remove", "Remove currently selected metric",   SLOT(removeButtonClickedSlot()));
	mRemoveAction->setDisabled(true);
	mLoadReferencePointsAction = this->createAction(group, ":/icons/metric_reference.png", "Import", "Import reference points from reference tool", SLOT(loadReferencePointsSlot()));
	mLoadReferencePointsAction->setDisabled(true);
	this->createAction(group, "", "", "", NULL)->setSeparator(true);
	mExportFramesAction = this->createAction(group, ":/icons/save.png", "ExportFrames", "Export metrics to file",   SLOT(exportMetricsButtonClickedSlot()));
}

//template<class T>
QAction* MetricWidget::createAction(QActionGroup* group, QString iconName, QString text, QString tip, const char* slot)
{
  QAction* action = new QAction(QIcon(iconName), text, group);
  action->setStatusTip(tip);
  action->setToolTip(tip);
  if (slot)
  {
  	connect(action, SIGNAL(triggered()), this, slot);
  }
  return action;
}

void MetricWidget::cellChangedSlot(int row, int col)
{
  if (col==0) // data name changed
  {
    QTableWidgetItem* item = mTable->item(row,col);
	DataPtr data = patientService()->getData(item->data(Qt::UserRole).toString());
    if (data)
      data->setName(item->text());
  }

}

void MetricWidget::cellClickedSlot(int row, int column)
{
	if (row < 0 || column < 0)
		return;

	  QTableWidgetItem* item = mTable->item(row,column);
	  QString uid = item->data(Qt::UserRole).toString();
	  mMetricManager->moveToMetric(uid);
}

void MetricWidget::itemSelectionChanged()
{
  QTableWidgetItem* item = mTable->currentItem();

  mMetricManager->setActiveUid(item->data(Qt::UserRole).toString());
  mEditWidgets->setCurrentIndex(mTable->currentRow());

  mMetricManager->setSelection(this->getSelectedUids());

  enablebuttons();
}

void MetricWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  this->setModified();
}

void MetricWidget::hideEvent(QHideEvent* event)
{
  QWidget::hideEvent(event);
}

namespace
{
template<class T, class SUPER>
boost::shared_ptr<T> castTo(boost::shared_ptr<SUPER> data)
{
	return boost::dynamic_pointer_cast<T>(data);
}

template<class T, class SUPER>
bool isType(boost::shared_ptr<SUPER> data)
{
	return (castTo<T>(data) ? true : false);
}

template<class WRAPPER, class METRIC, class SUPER>
boost::shared_ptr<WRAPPER> createMetricWrapperOfType(cx::ViewServicePtr viewService, cx::PatientModelServicePtr patientModelService, boost::shared_ptr<SUPER> data)
{
	return boost::shared_ptr<WRAPPER>(new WRAPPER(viewService, patientModelService, castTo<METRIC>(data)));
}
}

MetricBasePtr MetricWidget::createMetricWrapper(cx::ViewServicePtr viewService, cx::PatientModelServicePtr patientModelService, DataPtr data)
{
	if (isType<PointMetric>(data))
	  return createMetricWrapperOfType<PointMetricWrapper, PointMetric>(viewService, patientModelService, data);
	if (isType<DistanceMetric>(data))
	  return createMetricWrapperOfType<DistanceMetricWrapper, DistanceMetric>(viewService, patientModelService, data);
	if (isType<AngleMetric>(data))
	  return createMetricWrapperOfType<AngleMetricWrapper, AngleMetric>(viewService, patientModelService, data);
	if (isType<FrameMetric>(data))
	  return createMetricWrapperOfType<FrameMetricWrapper, FrameMetric>(viewService, patientModelService, data);
	if (isType<ToolMetric>(data))
	  return createMetricWrapperOfType<ToolMetricWrapper, ToolMetric>(viewService, patientModelService, data);
	if (isType<PlaneMetric>(data))
	  return createMetricWrapperOfType<PlaneMetricWrapper, PlaneMetric>(viewService, patientModelService, data);
	if (isType<DonutMetric>(data))
	  return createMetricWrapperOfType<DonutMetricWrapper, DonutMetric>(viewService, patientModelService, data);
	if (isType<SphereMetric>(data))
	  return createMetricWrapperOfType<SphereMetricWrapper, SphereMetric>(viewService, patientModelService, data);

	return MetricBasePtr();
}

/** create new metric wrappers for all metrics in PaSM
 *
 */
std::vector<MetricBasePtr> MetricWidget::createMetricWrappers(cx::ViewServicePtr viewService, cx::PatientModelServicePtr patientModelService)
{
	std::vector<MetricBasePtr> retval;
  std::map<QString, DataPtr> all = patientService()->getData();
  for (std::map<QString, DataPtr>::iterator iter=all.begin(); iter!=all.end(); ++iter)
  {
	MetricBasePtr wrapper = this->createMetricWrapper(viewService, patientModelService, iter->second);
  	if (wrapper)
  	{
  		retval.push_back(wrapper);
  	}
  }
  return retval;
}


void MetricWidget::prePaintEvent()
{
//	QTime timer;
//	timer.start();
	mPaintCount++;
  std::vector<MetricBasePtr> newMetrics = this->createMetricWrappers(mViewService, mPatientModelService);

  bool rebuild = !this->checkEqual(newMetrics, mMetrics);
  if (rebuild)
  {
	this->resetWrappersAndEditWidgets(newMetrics);
	this->initializeTable();
  }

  this->updateMetricWrappers();
  this->updateTableContents();

  if (rebuild)
  {
	  this->expensizeColumnResize();
  }

  this->enablebuttons();
//  std::cout << QString("prepaint, mod=%1, paint=%2, elapsed=%3ms").arg(mModifiedCount).arg(mPaintCount).arg(timer.elapsed()) << std::endl;
//  std::cout << QString("prepaint, mod=%1, paint=%2").arg(mModifiedCount).arg(mPaintCount) << std::endl;
}

void MetricWidget::expensizeColumnResize()
{
	int valueColumn = 1;
	mTable->resizeColumnToContents(valueColumn);
}

void MetricWidget::initializeTable()
{
	mTable->blockSignals(true);

	mTable->clear();

	mTable->setRowCount(mMetrics.size());
	mTable->setColumnCount(4);
	QStringList headerItems(QStringList() << "Name" << "Value" << "Arguments" << "Type");
	mTable->setHorizontalHeaderLabels(headerItems);
//	mTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents); // dangerous: uses lots of painting time
	mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	mTable->verticalHeader()->hide();

	for (unsigned i = 0; i < mMetrics.size(); ++i)
	{
		MetricBasePtr current = mMetrics[i];

		for (unsigned j = 0; j < 4; ++j)
		{
			QTableWidgetItem* item = new QTableWidgetItem("empty");
			item->setData(Qt::UserRole, current->getData()->getUid());
			mTable->setItem(i, j, item);
		}
	}
	mTable->blockSignals(false);
}

void MetricWidget::updateMetricWrappers()
{
	for (unsigned i = 0; i < mMetrics.size(); ++i)
	{
		mMetrics[i]->update();
	}
}

void MetricWidget::updateTableContents()
{
	mTable->blockSignals(true);
	// update contents:
	QTime timer;
	timer.start();
	for (unsigned i = 0; i < mMetrics.size(); ++i)
	{
		MetricBasePtr current = mMetrics[i];
		QString name = current->getData()->getName();
		QString value = current->getValue();
		QString arguments = current->getArguments();
		QString type = current->getType();
	}

	for (unsigned i = 0; i < mMetrics.size(); ++i)
	{
		MetricBasePtr current = mMetrics[i];
		if (!mTable->item(i,0))
		{
			std::cout << "no qitem for:: " << i << " " << current->getData()->getName() << std::endl;
			continue;
		}
		QString name = current->getData()->getName();
		QString value = current->getValue();
		QString arguments = current->getArguments();
		QString type = current->getType();

		mTable->item(i,0)->setText(name);
		mTable->item(i,1)->setText(value);
		mTable->item(i,2)->setText(arguments);
		mTable->item(i,3)->setText(type);

		//highlight selected row
		if (current->getData()->getUid() == mMetricManager->getActiveUid())
		{
			mTable->setCurrentCell(i,1);
			mEditWidgets->setCurrentIndex(i);
		}
	}
	mTable->blockSignals(false);
}

void MetricWidget::setModified()
{
	mLocalModified = true;
//	BaseWidget::setModified();
	mModifiedCount++;
}

void MetricWidget::delayedUpdate()
{
	if (!mLocalModified)
		return;
	BaseWidget::setModified();
	mLocalModified = false;

}

void MetricWidget::resetWrappersAndEditWidgets(std::vector<MetricBasePtr> wrappers)
{
	while (mEditWidgets->count())
	{
		mEditWidgets->removeWidget(mEditWidgets->widget(0));
	}

	for (unsigned i=0; i<mMetrics.size(); ++i)
	{
		disconnect(mMetrics[i]->getData().get(), SIGNAL(transformChanged()), this, SLOT(setModified()));
	}

	mMetrics = wrappers;

	for (unsigned i=0; i<mMetrics.size(); ++i)
	{
		connect(mMetrics[i]->getData().get(), SIGNAL(transformChanged()), this, SLOT(setModified()));
	}

	for (unsigned i=0; i<mMetrics.size(); ++i)
	{
		MetricBasePtr wrapper = mMetrics[i];
		QGroupBox* groupBox = new QGroupBox(wrapper->getData()->getName(), this);
		groupBox->setFlat(true);
		QVBoxLayout* gbLayout = new QVBoxLayout(groupBox);
		gbLayout->setMargin(4);
		gbLayout->addWidget(wrapper->createWidget());
		mEditWidgets->addWidget(groupBox);
	}

	mEditWidgets->setCurrentIndex(-1);
}

bool MetricWidget::checkEqual(const std::vector<MetricBasePtr>& a, const std::vector<MetricBasePtr>& b) const
{
	if (a.size()!=b.size())
		return false;

	for (unsigned i=0; i<a.size(); ++i)
	{
		if (a[i]->getData()!=b[i]->getData())
			return false;
	}

	return true;
}

void MetricWidget::enablebuttons()
{
	mRemoveAction->setEnabled(!mMetricManager->getActiveUid().isEmpty());
  mLoadReferencePointsAction->setEnabled(trackingService()->getReferenceTool() ? true : false);
}

void MetricWidget::loadReferencePointsSlot()
{
	mMetricManager->loadReferencePointsSlot();
}
void MetricWidget::addPointButtonClickedSlot()
{
	mMetricManager->addPointButtonClickedSlot();
}
void MetricWidget::addFrameButtonClickedSlot()
{
	mMetricManager->addFrameButtonClickedSlot();
}
void MetricWidget::addToolButtonClickedSlot()
{
	mMetricManager->addToolButtonClickedSlot();
}
void MetricWidget::addPlaneButtonClickedSlot()
{
	mMetricManager->addPlaneButtonClickedSlot();
}
void MetricWidget::addAngleButtonClickedSlot()
{
	mMetricManager->addAngleButtonClickedSlot();
}
void MetricWidget::addDistanceButtonClickedSlot()
{
	mMetricManager->addDistanceButtonClickedSlot();
}
void MetricWidget::addSphereButtonClickedSlot()
{
	mMetricManager->addSphereButtonClickedSlot();
}
void MetricWidget::addDonutButtonClickedSlot()
{
	mMetricManager->addDonutButtonClickedSlot();
}

std::set<QString> MetricWidget::getSelectedUids()
{
	QList<QTableWidgetItem*> selection = mTable->selectedItems();

	std::set<QString> selectedUids;
	for (int i=0; i<selection.size(); ++i)
	{
	  selectedUids.insert(selection[i]->data(Qt::UserRole).toString());
	}
	return selectedUids;
}

void MetricWidget::removeButtonClickedSlot()
{
	int nextIndex = mTable->currentRow() + 1;
	QString nextUid;
	if (nextIndex < mTable->rowCount())
	{
		QTableWidgetItem* nextItem = mTable->item(nextIndex, 0);
		nextUid = nextItem->data(Qt::UserRole).toString();
	}

	mPatientModelService->removeData(mMetricManager->getActiveUid());

	if (!nextUid.isEmpty())
		mMetricManager->setActiveUid(nextUid);
}

void MetricWidget::exportMetricsButtonClickedSlot()
{
	QString suggestion = QString("%1/Logs/metrics_%2.txt")
			.arg(patientService()->getActivePatientFolder())
			.arg(QDateTime::currentDateTime().toString(timestampSecondsFormat()));

	QString filename = QFileDialog::getSaveFileName(this,
													"Create/select file to export metrics to",
													suggestion);
	if(!filename.isEmpty())
		mMetricManager->exportMetricsToFile(filename);
}



}//end namespace cx
