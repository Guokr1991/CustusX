#include "sscManualTool.h"
#include <QTime>
#include <QTimer>
#include <vtkSTLReader.h>
#include <vtkCursor3D.h>
#include "sscDummyTool.h"
#include "sscMessageManager.h"

namespace ssc
{

ManualTool::ManualTool(const QString& uid, const QString& name) :
    Tool(uid,name), mMutex(QMutex::Recursive)
{
  m_prMt = Transform3D::Identity();
	mOffset = 0;
	mType = TOOL_MANUAL;
	mVisible = false;
	read3DCrossHair();
}

ManualTool::~ManualTool()
{
}

void ManualTool::read3DCrossHair()
{
	if(!mCrossHair)
	{
	  mCrossHair = vtkCursor3DPtr::New();
	  mCrossHair->AllOff();
	  mCrossHair->AxesOn();
	}
	int s = 60;
	mCrossHair->SetModelBounds(-s,s,-s,s,-s,s+mOffset);
	mCrossHair->SetFocalPoint(0,0,mOffset);
	mCrossHair->Modified();
}

//only used now by mouse or touch tool
//copied into cx::Tool, move to ssc::Tool?
void ManualTool::set_prMt(const Transform3D& prMt)
{
	QDateTime time;
	double timestamp = (double) time.time().msec();

	QMutexLocker locker(&mMutex);
	m_prMt = prMt;
	locker.unlock();

	emit toolTransformAndTimestamp(prMt, timestamp);
}

QString ManualTool::getGraphicsFileName() const
{
	return "";
}

ssc::Tool::Type ManualTool::getType() const
{
	return mType;
}

vtkPolyDataPtr ManualTool::getGraphicsPolyData() const
{
	return mCrossHair->GetOutput();
}

Transform3D ManualTool::get_prMt() const
{
	QMutexLocker locker(&mMutex);
	return m_prMt;
}

bool ManualTool::getVisible() const
{
	QMutexLocker locker(&mMutex);
	return mVisible;
}

int ManualTool::getIndex() const
{
	return -1;
}

QString ManualTool::getUid() const
{
	return mUid;
}

QString ManualTool::getName() const
{
	return mName;
}

void ManualTool::setVisible(bool vis)
{
	QMutexLocker locker(&mMutex);
	if (mVisible==vis)
	  return;
	mVisible = vis;
	emit toolVisible(mVisible);
}

void ManualTool::setType(const Type& type)
{
	QMutexLocker locker(&mMutex);
	mType = type;
}

bool ManualTool::isCalibrated() const
{
	return false;
}

ssc::ProbeData ManualTool::getProbeSector() const
{
	return mSector;
}

void ManualTool::setProbeSector(ssc::ProbeData sector)
{
	mSector = sector;
	emit toolProbeSector();
}

double ManualTool::getTimestamp() const
{
	return 0;
}

double ManualTool::getTooltipOffset() const
{
	return mOffset;
}

void ManualTool::setTooltipOffset(double val)
{
	if (similar(val,mOffset))
		return;
	mOffset = val;
	read3DCrossHair();
	emit tooltipOffset(mOffset);
}

ssc::Transform3D ManualTool::getCalibration_sMt() const
{
	return Transform3D::Identity();
}

}//end namespace
