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

#ifndef CXVIEW_H_
#define CXVIEW_H_

#include "sscConfig.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "vtkForwardDeclarations.h"
#include "cxIndent.h"
#include <QSize>
class QColor;

#include "cxTransform3D.h"

namespace cx
{
class DoubleBoundingBox3D;
typedef boost::shared_ptr<class Rep> RepPtr;

/**\brief Base widget for displaying lists of Rep.
 *
 * View inherits from QWidget and thus can be added to a QLayout.
 * It wraps vtkRenderer and vtkRenderWindow, and visualizes a scene
 * with one camera. Add reps to the View in order to visualize them.
 * Although any Rep can be added to a View, it makes most sense
 * dedicate each view for either 2D, 3D or Video display. Most Reps
 * are specialized to one of these three modes.
 *
 * Note: Some special hacks has been introduced in order to share
 * GPU memory between GL contexts (i.e. Views). This is described
 * in the vtkMods folder.
 *
 * \ingroup cx_resource_visualization
 */
class View : public QObject
{
	Q_OBJECT
public:
	/// type describing the view
	enum Type
	{
		VIEW, VIEW_2D, VIEW_3D, VIEW_REAL_TIME
	};
	virtual ~View() {}
	virtual Type getType() const = 0;
	virtual QString getTypeString() const = 0;
	virtual QString getUid() = 0; ///< Get a views unique id
	virtual QString getName() = 0; ///< Get a views name
	virtual vtkRendererPtr getRenderer() const = 0; ///< Get the renderer used by this \a View.
	virtual void addRep(const RepPtr& rep) = 0; ///< Adds and connects a rep to the view
//	virtual void setRep(const RepPtr& rep) = 0; ///< Remove all other \a Rep objects from this \a View and add the provided Rep to this \a View.
	virtual void removeRep(const RepPtr& rep) = 0; ///< Removes and disconnects the rep from the view
	virtual bool hasRep(const RepPtr& rep) const = 0; ///< Checks if the view already have the rep
	virtual std::vector<RepPtr> getReps() = 0; ///< Returns all reps in the view
	virtual void removeReps() = 0; ///< Removes all reps in the view
	virtual void setBackgroundColor(QColor color) = 0;
	virtual void render() = 0; ///< render the view contents if vtk-MTimes are changed
	virtual vtkRenderWindowPtr getRenderWindow() const = 0;
	virtual QSize size() const = 0;
	virtual void setZoomFactor(double factor) = 0;
	virtual double getZoomFactor() const = 0;
	virtual Transform3D get_vpMs() const = 0;
//	virtual double heightMM() const = 0;
	virtual DoubleBoundingBox3D getViewport() const = 0;
	virtual DoubleBoundingBox3D getViewport_s() const = 0;
	virtual void setModified() = 0;

signals:
	void resized(QSize size);
	void mouseMove(int x, int y, Qt::MouseButtons buttons);
	void mousePress(int x, int y, Qt::MouseButtons buttons);
	void mouseRelease(int x, int y, Qt::MouseButtons buttons);
	void mouseWheel(int x, int y, int delta, int orientation, Qt::MouseButtons buttons);
	void shown();
	void focusChange(bool gotFocus, Qt::FocusReason reason);
	void customContextMenuRequested(const QPoint&);

};
typedef boost::shared_ptr<View> ViewPtr;


typedef boost::shared_ptr<View> ViewPtr;

} // namespace cx

#endif /*CXVIEW_H_*/
