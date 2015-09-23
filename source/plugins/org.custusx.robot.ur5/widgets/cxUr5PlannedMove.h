#ifndef UR5PLANNEDMOVE_H
#define UR5PLANNEDMOVE_H

#include "org_custusx_robot_ur5_Export.h"
#include "cxUr5Robot.h"

#include <QPushButton>
#include <QLineEdit>

namespace cx
{

class org_custusx_robot_ur5_EXPORT Ur5PlannedMoveTab : public QWidget
{
    Q_OBJECT
public:
    Ur5PlannedMoveTab(Ur5RobotPtr Ur5Robot, QWidget *parent = 0);
    virtual ~Ur5PlannedMoveTab();

    QPushButton *runVTKButton,*goToOrigoButton;
    QLineEdit *vtkLineEdit;

    QLineEdit *accelerationLineEdit, *velocityLineEdit, *timeLineEdit;

protected slots:
    void runVTKfileSlot();
    void goToOrigoButtonSlot();

private:
    void setupUi(QWidget *parent);
    Ur5RobotPtr mUr5Robot;
};

} // cx

#endif // UR5PLANNEDMOVE_H