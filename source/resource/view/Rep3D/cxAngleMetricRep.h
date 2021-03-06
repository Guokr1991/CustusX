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


#ifndef CXANGLEMETRICREP_H_
#define CXANGLEMETRICREP_H_

#include "cxResourceVisualizationExport.h"

#include "cxDataMetricRep.h"
#include "vtkForwardDeclarations.h"
#include "cxForwardDeclarations.h"

typedef vtkSmartPointer<class vtkTextActor> vtkTextActorPtr;

namespace cx
{
typedef boost::shared_ptr<class CaptionText3D> CaptionText3DPtr;
typedef boost::shared_ptr<class GraphicalArc3D> GraphicalArc3DPtr;
}

namespace cx
{

typedef boost::shared_ptr<class AngleMetricRep> AngleMetricRepPtr;
typedef boost::shared_ptr<class AngleMetric> AngleMetricPtr;

/** Rep for visualizing a AngleMetric.
 *
 * \ingroup cx_resource_view
 * \ingroup cx_resource_view_rep3D
 *
 * \date Jul 27, 2011
 * \author Christian Askeland, SINTEF
 */
class cxResourceVisualization_EXPORT AngleMetricRep: public DataMetricRep
{
Q_OBJECT
public:
	static AngleMetricRepPtr New(const QString& uid = ""); ///constructor
	virtual ~AngleMetricRep() {}

	virtual QString getType() const { return "AngleMetricRep"; }

protected:
    virtual void clear();
	virtual void onModifiedStartRender();

private:
	AngleMetricRep(const QString& uid, const QString& name = "");
	AngleMetricRep(); ///< not implemented
    AngleMetricPtr getAngleMetric();

	GraphicalLine3DPtr mLine0;
	GraphicalLine3DPtr mLine1;
	GraphicalArc3DPtr mArc;
};

}

#endif /* CXANGLEMETRICREP_H_ */
