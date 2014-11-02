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

#include "cxManualToolAdapter.h"
#include "cxToolManager.h"

namespace cx
{

ManualToolAdapter::ManualToolAdapter(QString uid) :
				ManualTool(uid)
{
	mBase.reset(new ManualTool(uid + "base"));
	connect(mBase.get(), SIGNAL(toolProbeSector()), this, SIGNAL(toolProbeSector()));
}

ManualToolAdapter::ManualToolAdapter(ToolPtr base) :
				ManualTool(mBase->getUid() + "_manual"), mBase(base)
{
}

ManualToolAdapter::~ManualToolAdapter()
{
}

void ManualToolAdapter::setBase(ToolPtr base)
{
	disconnect(mBase.get(), SIGNAL(toolProbeSector()), this, SIGNAL(toolProbeSector()));
	mBase = base;
	connect(mBase.get(), SIGNAL(toolProbeSector()), this, SIGNAL(toolProbeSector()));

	emit toolProbeSector();
}

vtkPolyDataPtr ManualToolAdapter::getGraphicsPolyData() const
{
	return mBase->getGraphicsPolyData();
}

bool ManualToolAdapter::isCalibrated() const
{
	return mBase->isCalibrated();
}

ProbeDefinition ManualToolAdapter::getProbeSector() const
{
	return mBase->getProbeSector();
}

ProbePtr ManualToolAdapter::getProbe() const
{
	return mBase->getProbe();
}

Transform3D ManualToolAdapter::getCalibration_sMt() const
{
	return mBase->getCalibration_sMt();
}

std::map<int, Vector3D> ManualToolAdapter::getReferencePoints() const
{
	return mBase->getReferencePoints();
}

double ManualToolAdapter::getTooltipOffset() const
{
	return mBase->getTooltipOffset();
}

void ManualToolAdapter::setTooltipOffset(double val)
{
	mBase->setTooltipOffset(val);
}

std::set<Tool::Type> ManualToolAdapter::getTypes() const
{
	std::set<Tool::Type> retval = mBase->getTypes();
	retval.insert(Tool::TOOL_MANUAL);
	return retval;
}

}
