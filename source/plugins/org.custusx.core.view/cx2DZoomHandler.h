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

#ifndef CX2DZOOMHANDLER_H
#define CX2DZOOMHANDLER_H

#include "org_custusx_core_view_Export.h"

#include "cxViewGroupData.h"
#include <QMenu>

namespace cx
{
typedef boost::shared_ptr<class Zoom2DHandler> Zoom2DHandlerPtr;

/** 
 *
 * \ingroup org_custusx_core_view
 * \date 2014-02-26
 * \author christiana
 */
class org_custusx_core_view_EXPORT Zoom2DHandler : public QObject
{
	Q_OBJECT
public:
	Zoom2DHandler();

	void addActionsToMenu(QMenu* contextMenu);
	void setGroupData(ViewGroupDataPtr group);
	double getFactor();
	void setFactor(double factor);

private slots:
	void zoom2DActionSlot();
signals:
	void zoomChanged();
private:
	void setConnectivityFromType(QString type);
	QString getConnectivityType();
	void addConnectivityAction(QString type, QString text, QMenu *contextMenu);
	void set(SyncedValuePtr value);

	SyncedValuePtr mZoom2D;
	ViewGroupDataPtr mGroupData;
};


} // namespace cx



#endif // CX2DZOOMHANDLER_H
