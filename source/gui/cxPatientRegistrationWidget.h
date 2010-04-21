#ifndef CXPATIENTREGISTRATIONWIDGET_H_
#define CXPATIENTREGISTRATIONWIDGET_H_

#include "cxRegistrationWidget.h"

#include <sscImage.h>
#include <sscTransform3D.h>
#include "cxTool.h"

class QVBoxLayout;
class QComboBox;
class QTableWidget;
class QPushButton;
class QString;
class QLabel;
class QSlider;
class QGridLayout;
class QSpinBox;

namespace cx
{
typedef ssc::Transform3D Transform3D;
typedef boost::shared_ptr<ssc::Vector3D> Vector3DPtr;

/**
 * \class PatientRegistrationWidget
 *
 * \brief Widget used as a tab in the ContexDockWidget for patient registration.
 *
 * \date Feb 3, 2009
 * \author: Janne Beate Bakeng, SINTEF
 */
class PatientRegistrationWidget : public RegistrationWidget
{
  Q_OBJECT

public:
  PatientRegistrationWidget(QWidget* parent); ///< sets up layout and connects signals and slots
  virtual ~PatientRegistrationWidget(); ///< empty

protected slots:

  virtual void activeImageChangedSlot(); ///< listens to the datamanager for when the active image is changed
  void toolVisibleSlot(bool visible); ///< enables/disables the Sample Tool button
  void toolSampleButtonClickedSlot(); ///< reacts when the Sample Tool button is clicked
  void dominantToolChangedSlot(const std::string& uid); ///< set which tool to sample from
  void resetOffsetSlot(); ///< resets the patient registration offset to zero
  void setOffsetSlot(int value); ///< set the patient registration offset
  void activateManualRegistrationFieldSlot(); ///< activates the manuall offset functionality

protected:
  virtual void showEvent(QShowEvent* event); ///<updates internal info before showing the widget
  virtual void populateTheLandmarkTableWidget(ssc::ImagePtr image); ///< populates the table widget
//  void updateAccuracy(); ///< calculates accuracy for each landmark after a registration
  virtual ssc::LandmarkMap getTargetLandmarks() const;
  virtual ssc::Transform3D getTargetTransform() const;
  virtual void performRegistration();


  //std::vector<ssc::Landmark> getAllLandmarks() const;

  //gui
  //QVBoxLayout* mVerticalLayout; ///< vertical layout is used
  //QTableWidget* mLandmarkTableWidget; ///< the table widget presenting the landmarks
  QPushButton* mToolSampleButton; ///< the Sample Tool button
  QLabel* mOffsetLabel; ///< header label for the offset section
  QWidget* mOffsetWidget; ///< widget for offset functionality
  QGridLayout* mOffsetsGridLayout; ///< layout to put the offset objects into
  QLabel* mXLabel; ///< the text X
  QLabel* mYLabel; ///< the text Y
  QLabel* mZLabel; ///< the text Z
  QSlider* mXOffsetSlider; ///< slider for the user to manually adjust a patient registration with an offset
  QSlider* mYOffsetSlider; ///< slider for the user to manually adjust a patient registration with an offset
  QSlider* mZOffsetSlider; ///< slider for the user to manually adjust a patient registration with an offset
  QSpinBox* mXSpinBox; ///< box for showing X value
  QSpinBox* mYSpinBox; ///< box for showing Y value
  QSpinBox* mZSpinBox; ///< box for showing Z value
  QPushButton* mResetOffsetButton; ///< button for resetting the offset to zero

  //data
  //std::string mActiveLandmark;
  //int mCurrentRow, mCurrentColumn; ///< which row and column are currently the choose ones
  //std::map<std, double> mLandmarkRegistrationAccuracyMap; ///< maps accuracy to index of a landmark
  //double mAverageRegistrationAccuracy; ///< the average registration accuracy of the last registration
  ssc::ToolPtr mToolToSample; ///< tool to be sampled from
  //ssc::ImagePtr mCurrentImage; ///< the image currently used in image registration

  int mMinValue, mMaxValue, mDefaultValue; ///< values for the range of the offset

private:
  PatientRegistrationWidget(); ///< not implemented

};
}//namespace cx

#endif /* CXPATIENTREGISTRATIONWIDGET_H_ */
