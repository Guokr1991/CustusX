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

#ifndef CXVISUALIZATIONIMPLSERVICE_H_
#define CXVISUALIZATIONIMPLSERVICE_H_

#include "cxVisualizationService.h"
#include "org_custusx_core_visualization_Export.h"
class ctkPluginContext;

namespace cx
{
typedef boost::shared_ptr<class ViewManager> VisualizationServiceOldPtr;

/**
 * Implementation of VisualizationService.
 *
 * \ingroup org_custusx_core_visualization
 *
 * \date 2014-09-19
 * \author Ole Vegard Solberg
 */
class org_custusx_core_visualization_EXPORT VisualizationImplService : public VisualizationService
{
	Q_INTERFACES(cx::VisualizationService)
public:
	VisualizationImplService(ctkPluginContext* context);
	virtual ~VisualizationImplService();

	virtual ViewPtr get3DView(int group = 0, int index = 0);

	virtual int getActiveGroup() const;
	virtual ViewGroupDataPtr getGroup(int groupIdx) const;

	virtual void autoShowData(DataPtr data);
	virtual void enableRender(bool val);
	virtual bool renderingIsEnabled() const;

	virtual QWidget* getLayoutWidget(QWidget *parent, int index);
	virtual QString getActiveLayout(int widgetIndex) const;
	virtual void setActiveLayout(const QString& uid, int widgetIndex);
	virtual InteractiveClipperPtr getClipper();
	virtual InteractiveCropperPtr getCropper();
	virtual CyclicActionLoggerPtr getRenderTimer();
	virtual NavigationPtr getNavigation();
	virtual LayoutRepositoryPtr getLayoutRepository();
	virtual CameraControlPtr getCameraControl();
	virtual QActionGroup* createInteractorStyleActionGroup();
	virtual void setPreview(ImagePtr image, const std::vector<double>& threshold);
	virtual void removePreview();
	virtual void clear();
	virtual void addXml(QDomNode& parentNode);
	virtual void parseXml(QDomNode viewmanagerNode);

	virtual bool isNull();

private:
	ctkPluginContext *mContext;
	VisualizationServiceOldPtr mBase;
	ViewManager* viewManager() const { return mBase.get(); }
};
typedef boost::shared_ptr<VisualizationImplService> VisualizationImplServicePtr;

} /* namespace cx */

#endif /* CXVISUALIZATIONIMPLSERVICE_H_ */

