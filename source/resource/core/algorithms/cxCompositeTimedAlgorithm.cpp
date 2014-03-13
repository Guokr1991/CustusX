// This file is part of CustusX, an Image Guided Therapy Application.
//
// Copyright (C) 2008- SINTEF Technology & Society, Medical Technology
//
// CustusX is fully owned by SINTEF Medical Technology (SMT). CustusX source
// code and binaries can only be used by SMT and those with explicit permission
// from SMT. CustusX shall not be distributed to anyone else.
//
// CustusX is a research tool. It is NOT intended for use or certified for use
// in a normal clinical setting. SMT does not take responsibility for its use
// in any way.
//
// See CustusX_License.txt for more information.

#include "cxCompositeTimedAlgorithm.h"

#include <QStringList>
#include "cxTypeConversions.h"
#include "cxMessageManager.h"

namespace cx
{

CompositeTimedAlgorithm::CompositeTimedAlgorithm(QString name) :
	TimedBaseAlgorithm(name, 20)
{
}

void CompositeTimedAlgorithm::append(TimedAlgorithmPtr child)
{
	mChildren.push_back(child);
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------


CompositeSerialTimedAlgorithm::CompositeSerialTimedAlgorithm(QString name) :
	CompositeTimedAlgorithm(name),
	mCurrent(-1)
{
}

QString CompositeSerialTimedAlgorithm::getProduct() const
{
	if (mCurrent >= 0 && mCurrent < mChildren.size())
	{
		return mChildren[mCurrent]->getProduct();
	}
	return "composite";
}

void CompositeSerialTimedAlgorithm::clear()
{
	// if already started, ignore
	if (mCurrent>=0)
	{
		messageManager()->sendError("Attempt to restart CompositeSerialTimedAlgorithm while running failed.");
		return;
	}

	mChildren.clear();
	mCurrent = -1;
}

void CompositeSerialTimedAlgorithm::execute()
{
	// if already started, ignore
	if (mCurrent>=0)
		return;
	this->startTiming();
	mCurrent = -1;
	emit started(0);
	this->jumpToNextChild();
}

void CompositeSerialTimedAlgorithm::jumpToNextChild()
{
	// teardown old child
	if (mCurrent >= 0 && mCurrent < mChildren.size())
	{
		disconnect(mChildren[mCurrent].get(), SIGNAL(finished()), this, SLOT(jumpToNextChild()));
	}
	++mCurrent;
	// setup and run next child
	if (mCurrent >= 0 && mCurrent < mChildren.size())
	{
		connect(mChildren[mCurrent].get(), SIGNAL(finished()), this, SLOT(jumpToNextChild()));
		emit productChanged();
		mChildren[mCurrent]->execute();
	}

	// check for finished
	if (mCurrent>=mChildren.size())
	{
		mCurrent = -1;
		this->stopTiming();
		emit finished();
	}
}

bool CompositeSerialTimedAlgorithm::isFinished() const
{
    return mCurrent == -1;
}

bool CompositeSerialTimedAlgorithm::isRunning() const
{
    return mCurrent >= 0;
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

CompositeParallelTimedAlgorithm::CompositeParallelTimedAlgorithm(QString name) :
	CompositeTimedAlgorithm(name)
{
}

QString CompositeParallelTimedAlgorithm::getProduct() const
{
	QStringList products;
	for (unsigned i=0; i<mChildren.size(); ++i)
	{
		products << mChildren[i]->getProduct();
	}

	if (products.isEmpty())
		return "composite parallel";

	return products.join(", ");
}

void CompositeParallelTimedAlgorithm::clear()
{

	// if already started, ignore
	if (!this->isFinished())
	{
		messageManager()->sendError("Attempt to restart CompositeSerialTimedAlgorithm while running failed.");
		return;
	}

	mChildren.clear();
}

void CompositeParallelTimedAlgorithm::execute()
{
	emit aboutToStart();
	emit started(0);
	for (unsigned i=0; i<mChildren.size(); ++i)
	{
		connect(mChildren[i].get(), SIGNAL(finished()), this, SLOT(oneFinished()));
	}
	for (unsigned i=0; i<mChildren.size(); ++i)
	{
		mChildren[i]->execute();
	}
}

void CompositeParallelTimedAlgorithm::oneFinished()
{
	emit productChanged();

	if (this->isFinished())
		emit finished();
}

bool CompositeParallelTimedAlgorithm::isFinished() const
{
	int count = 0;
	for (unsigned i=0; i<mChildren.size(); ++i)
	{
		if (mChildren[i]->isFinished())
			++count;
	}
	return (count==mChildren.size());
}

bool CompositeParallelTimedAlgorithm::isRunning() const
{
    return !this->isFinished();
}



}
