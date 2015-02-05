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

#ifndef CXSTREAMREP3D_H
#define CXSTREAMREP3D_H

#include "cxVolumetricRep.h"
#include "cxForwardDeclarations.h"
#include "cxTransform3D.h"

namespace cx
{
typedef boost::shared_ptr<class VideoSourceGraphics> VideoSourceGraphicsPtr;

/** \brief Display a stream (2D/3D) in 3D
 *
 * \ingroup cx_resource_visualization
 * \ingroup cx_resource_visualization_rep3D
 *
 * \date jan 29, 2015
 * \author Ole Vegard Solberg, SINTEF
 */
class cxResourceVisualization_EXPORT StreamRep3D : public VolumetricRep
{
	Q_OBJECT
public:
	static StreamRep3DPtr New(SpaceProviderPtr spaceProvider, PatientModelServicePtr patientModelService, const QString &uid="");
	virtual QString getType() const;

	void setTrackedStream(TrackedStreamPtr trackedStream);
	TrackedStreamPtr getTrackedStream();

private slots:
	void newTool(ToolPtr tool);
	void newVideoSource(VideoSourcePtr videoSource);
	void newFrame();
private:
	StreamRep3D(SpaceProviderPtr spaceProvider, PatientModelServicePtr patientModelService);

	TrackedStreamPtr mTrackedStream;
	VideoSourcePtr mVideoSource;

	PatientModelServicePtr mPatientModelService;
	void initTransferFunction(ImagePtr image);
};

} //cx

#endif // CXSTREAMREP3D_H
