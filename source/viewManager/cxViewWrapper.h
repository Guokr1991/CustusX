#ifndef CXVIEWWRAPPER_H_
#define CXVIEWWRAPPER_H_

#include <QObject>
#include "sscImage.h"
#include "sscView.h"
#include "sscDefinitions.h"

class QMenu;
class QActionGroup;

namespace cx
{

/**
 * \class cxViewWrapper.h
 *
 * \brief
 *
 * \date 6. apr. 2010
 * \author: jbake
 */
class ViewWrapper : public QObject
{
  Q_OBJECT
public:
  virtual ~ViewWrapper() {}
  virtual void initializePlane(ssc::PLANE_TYPE plane) {}
  virtual void setImage(ssc::ImagePtr image) = 0;
  virtual void removeImage(ssc::ImagePtr image) = 0;
  virtual void setRegistrationMode(ssc::REGISTRATION_STATUS mode) {}
  virtual ssc::View* getView() = 0;
  virtual void setZoom2D(double zoomFactor) {}

signals:
  void zoom2DChange(double newZoom);

protected slots:
  void contextMenuSlot(const QPoint& point);

protected:
  //void connectContextMenu();
  void connectContextMenu(ssc::View* view);
  virtual void appendToContextMenu(QMenu& contextMenu) = 0;
  virtual void checkFromContextMenu(QAction* theAction, QActionGroup* theActionGroup) = 0;
};
typedef boost::shared_ptr<ViewWrapper> ViewWrapperPtr;

}//namespace cx
#endif /* CXVIEWWRAPPER_H_ */
