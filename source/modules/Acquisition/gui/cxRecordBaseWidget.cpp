#include "cxRecordBaseWidget.h"

#include <QPushButton>
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <vtkPolyData.h>
#include "cxToolManager.h"
#include "cxReporter.h"
#include "cxDataManager.h"
#include "cxMesh.h"
#include "cxTransform3D.h"
#include "cxRecordSessionWidget.h"
#include "cxVideoConnectionManager.h"
#include "cxProbeImpl.h"
#include "cxSoundSpeedConversionWidget.h"

namespace cx
{

RecordBaseWidget::RecordBaseWidget(AcquisitionDataPtr pluginData, QWidget* parent, QString description):
    BaseWidget(parent, "RecordBaseWidget", "Record Base"),
    mPluginData(pluginData),
    mLayout(new QVBoxLayout(this))
{
	this->setObjectName("RecordBaseWidget");
	this->setWindowTitle("Record Base");

	mBase.reset(new Acquisition(pluginData));
	mRecordSessionWidget = new RecordSessionWidget(mBase, this, description);
	mLayout->addWidget(mRecordSessionWidget);
}

RecordBaseWidget::~RecordBaseWidget()
{}

////----------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------------------------------------

TrackedRecordWidget::TrackedRecordWidget(AcquisitionDataPtr pluginData, QWidget* parent, QString description) :
  RecordBaseWidget(pluginData, parent, description)
{}

TrackedRecordWidget::~TrackedRecordWidget()
{}

void TrackedRecordWidget::setTool(ToolPtr tool)
{
  if(mTool && tool && (mTool->getUid() == tool->getUid()))
    return;

  mTool = tool;
  emit toolChanged();
}

ToolPtr TrackedRecordWidget::getTool()
{
  return mTool;
}
//----------------------------------------------------------------------------------------------------------------------
}//namespace cx