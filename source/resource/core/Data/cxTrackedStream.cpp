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

#include "cxTrackedStream.h"

#include <vtkImageData.h>

#include "cxTool.h"
#include "cxRegistrationTransform.h"

namespace cx
{

TrackedStreamPtr TrackedStream::create(const QString &uid, const QString &name)
{
	return TrackedStreamPtr(new TrackedStream(uid, name, ToolPtr(), VideoSourcePtr()));
}

TrackedStream::TrackedStream(const QString& uid, const QString& name, const ToolPtr &probe, const VideoSourcePtr &videosource) :
	Data(uid, name), mProbeTool(probe), //mVideoSource(videosource)
	mImage(ImagePtr())
{
	if(mProbeTool)
		emit newTool(mProbeTool);

	setVideoSource(videosource);
}

void TrackedStream::setProbeTool(const ToolPtr &probeTool)
{
	if(mProbeTool)
		disconnect(mProbeTool.get(), &Tool::toolTransformAndTimestamp, this, &TrackedStream::toolTransformAndTimestamp);

	mProbeTool = probeTool;
	emit newTool(mProbeTool);

	if(mProbeTool)
		connect(mProbeTool.get(), &Tool::toolTransformAndTimestamp, this, &TrackedStream::toolTransformAndTimestamp);
}

void TrackedStream::toolTransformAndTimestamp(Transform3D prMt, double timestamp)
{
//	std::cout << "TrackedStream::toolTransformAndTimestamp prMt: " << prMt << std::endl;
	if (mImage)
		mImage->get_rMd_History()->setRegistration(prMt);
}

ToolPtr TrackedStream::getProbeTool()
{
	return mProbeTool;
}

void TrackedStream::setVideoSource(const VideoSourcePtr &videoSource)
{
	if(mVideoSource)
		disconnect(mVideoSource.get(), &VideoSource::newFrame, this, &TrackedStream::newFrame);

	mVideoSource = videoSource;
	emit streamChanged();
	emit newVideoSource(mVideoSource);

	if(mVideoSource)
		connect(mVideoSource.get(), &VideoSource::newFrame, this, &TrackedStream::newFrameSlot);
}

void TrackedStream::newFrameSlot()
{
	//TODO: Check if we need to turn this on/off
	if (mImage && mVideoSource)
	{
		mImage->setVtkImageData(mVideoSource->getVtkImageData(), false);
		emit newFrame();
	}
}

VideoSourcePtr TrackedStream::getVideoSource()
{
	return mVideoSource;
}

void TrackedStream::addXml(QDomNode &dataNode)
{
	Data::addXml(dataNode);
}

void TrackedStream::parseXml(QDomNode &dataNode)
{
	Data::parseXml(dataNode);
}

DoubleBoundingBox3D TrackedStream::boundingBox() const
{
	DoubleBoundingBox3D bounds(mVideoSource->getVtkImageData()->GetBounds());
	return bounds;
}

QString TrackedStream::getType() const
{
	return getTypeName();
}

QString TrackedStream::getTypeName()
{
	return "TrackedStream";
}

ImagePtr TrackedStream::getChangingImage()
{
	if(!mVideoSource)
		return ImagePtr();
	if (!mImage)
		mImage = ImagePtr(new Image(this->getUid()+"_TrackedStreamHelper", mVideoSource->getVtkImageData(), this->getName()+"_TrackedStreamHelper"));
	return mImage;
}

} //cx
