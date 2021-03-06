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
#ifndef CXSPHEREMETRICREP2D_H
#define CXSPHEREMETRICREP2D_H

#include "cxResourceVisualizationExport.h"

#include "cxDataMetricRep.h"
#include "cxGraphicalPrimitives.h"
#include "cxPointMetric.h"
#include "cxViewportListener.h"

namespace cx
{

typedef boost::shared_ptr<class GraphicalDisk> GraphicalDiskPtr;
typedef boost::shared_ptr<class SphereMetricRep2D> SphereMetricRep2DPtr;
typedef boost::shared_ptr<class SphereMetric> SphereMetricPtr;

/**
 *
 * \ingroup cx_resource_view
 * \ingroup cx_resource_view_rep2D
 * \date 2014-03-27
 * \author christiana
 */
class cxResourceVisualization_EXPORT SphereMetricRep2D: public DataMetricRep
{
Q_OBJECT
public:
	static SphereMetricRep2DPtr New(const QString& uid=""); ///constructor
	virtual ~SphereMetricRep2D() {}

	virtual QString getType() const { return "SphereMetricRep2D"; }
	void setSliceProxy(SliceProxyPtr slicer);

protected:
	virtual void addRepActorsToViewRenderer(ViewPtr view);
	virtual void removeRepActorsFromViewRenderer(ViewPtr view);

	virtual void clear();
	virtual void onModifiedStartRender();

private:
	SphereMetricRep2D();

	SphereMetricPtr getSphereMetric();

	SliceProxyPtr mSliceProxy;
	GraphicalDiskPtr mDisk;
};


} // namespace cx


#endif // CXSPHEREMETRICREP2D_H
