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

#ifndef CXVIEWWRAPPERRTSTREAM_H_
#define CXVIEWWRAPPERRTSTREAM_H_

#include <vector>
#include <QPointer>
#include "cxForwardDeclarations.h"
#include "cxDefinitions.h"

#include "cxViewWrapper.h"

namespace cx
{
/**
* \file
* \addtogroup cx_service_visualization
* @{
*/

/** Wrapper for a View that displays a RealTimeStream.
 *  Handles the connections between specific reps and the view.
 *
 *  The view displays either a raw rt source or a us probe, depending on
 *  whats available.
 *
 */
class ViewWrapperVideo: public ViewWrapper
{
Q_OBJECT
public:
	ViewWrapperVideo(ViewWidget* view, VisualizationServiceBackendPtr backend);
	virtual ~ViewWrapperVideo();
	virtual ViewWidget* getView();
	virtual void setSlicePlanesProxy(SlicePlanesProxyPtr proxy) {}
	virtual void updateView() {}
	virtual void setViewGroup(ViewGroupDataPtr group);

private slots:
	void updateSlot();
	void showSectorActionSlot(bool checked);
	void connectStream();
	void streamActionSlot();
protected slots:
	virtual void dataViewPropertiesChangedSlot(QString uid) {}

protected:
//	virtual void dataAdded(DataPtr data) {}
//	virtual void dataRemoved(const QString& uid) {}
	virtual void videoSourceChangedSlot(QString uid);

private:
	VideoSourcePtr getSourceFromService(QString uid);
	void addStreamAction(QString uid, QMenu* contextMenu);
	void loadStream();
	virtual void appendToContextMenu(QMenu& contextMenu);
	void addReps();
	void setupRep(VideoSourcePtr source, ToolPtr tool);

	VideoFixedPlaneRepPtr mStreamRep;
	VideoSourcePtr mSource;
	DisplayTextRepPtr mPlaneTypeText;
	DisplayTextRepPtr mDataNameText;
	QPointer<ViewWidget> mView;
	ToolPtr mTool;
};
typedef boost::shared_ptr<ViewWrapperVideo> ViewWrapperVideoPtr;

/**
* @}
*/
} // namespace cx

#endif /* CXVIEWWRAPPERRTSTREAM_H_ */
