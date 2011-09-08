#include "cxViewWrapper3D.h"

#include <vector>

#include "boost/bind.hpp"
#include "boost/function.hpp"

#include <QAction>
#include <QMenu>

#include <vtkTransform.h>
#include <vtkCamera.h>
#include <vtkAbstractVolumeMapper.h>
#include <vtkVolumeMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkInteractorObserver.h>

#include "sscView.h"
#include "sscSliceProxy.h"
#include "sscSlicerRepSW.h"
#include "sscTool2DRep.h"
#include "sscOrientationAnnotationRep.h"
#include "sscDisplayTextRep.h"
#include "sscMessageManager.h"
#include "sscSlicePlanes3DRep.h"
#include "sscDataManager.h"
#include "sscMesh.h"
#include "sscProbeRep.h"
#include "sscGeometricRep.h"
#include "sscToolRep3D.h"
#include "sscVolumetricRep.h"
#include "sscTypeConversions.h"
#include "sscVideoSource.h"
#include "sscVideoRep.h"
#include "sscToolTracer.h"
#include "sscOrientationAnnotation3DRep.h"
#include "cxSettings.h"
#include "cxToolManager.h"
#include "cxRepManager.h"
#include "cxCameraControl.h"
#include "cxImageLandmarkRep.h"
#include "cxPatientLandmarkRep.h"
#include "cxPointMetricRep.h"
#include "cxDistanceMetricRep.h"
#include "cxAngleMetricRep.h"
#include "cxPlaneMetricRep.h"
#include "cxDataMetricRep.h"
#include "cxDataLocations.h"

namespace cx
{

ssc::AxesRepPtr ToolAxisConnector::getAxis_t()
{
	return mAxis_t;
}

ssc::AxesRepPtr ToolAxisConnector::getAxis_s()
{
	return mAxis_s;
}

ToolAxisConnector::ToolAxisConnector(ssc::ToolPtr tool)
{
	mTool = tool;
	mAxis_s = ssc::AxesRep::New(tool->getUid()+"_axis_s");
	mAxis_t = ssc::AxesRep::New(tool->getUid()+"_axis_t");

	mAxis_t->setAxisLength(0.08);
	mAxis_t->setShowAxesLabels(false);
	//mAxis_t->setCaption(tool->getName()+"_t", ssc::Vector3D(1,1,0.7));
	mAxis_t->setCaption("t", ssc::Vector3D(0.7,1,0.7));
	mAxis_t->setFontSize(0.03);

	mAxis_s->setAxisLength(0.05);
	mAxis_s->setShowAxesLabels(false);
	mAxis_s->setCaption("s", ssc::Vector3D(1,1,0));
	mAxis_s->setFontSize(0.03);

	connect(mTool.get(), SIGNAL(toolTransformAndTimestamp(Transform3D, double)), this, SLOT(transformChangedSlot()));
	connect(mTool.get(), SIGNAL(toolVisible(bool)), this, SLOT(visibleSlot()));
	visibleSlot();
	transformChangedSlot();
}

void ToolAxisConnector::transformChangedSlot()
{
	ssc::Transform3D rMt = *ssc::toolManager()->get_rMpr() * mTool->get_prMt();
	ssc::Transform3D sMt = mTool->getCalibration_sMt();
	mAxis_t->setTransform(rMt);
	mAxis_s->setTransform(rMt*sMt.inv());
}

void ToolAxisConnector::visibleSlot()
{
	mAxis_t->setVisible(mTool->getVisible());
	mAxis_s->setVisible(mTool->getVisible());
	if (ssc::similar(mTool->getCalibration_sMt(), ssc::Transform3D::Identity()))
	{
		mAxis_s->setVisible(false);
	}
}

///--------------------------------------------------------
///--------------------------------------------------------

///--------------------------------------------------------
///--------------------------------------------------------


ViewWrapper3D::ViewWrapper3D(int startIndex, ssc::View* view)
{
  mShowAxes = false;
  mView = view;
  this->connectContextMenu(mView);
  QString index = QString::number(startIndex);
//  connect(ssc::dataManager(), SIGNAL(centerChanged()), this, SLOT(centerChangedSlot()));
  QColor background = settings()->value("backgroundColor").value<QColor>();
  mView->setBackgoundColor(background);

  view->getRenderer()->GetActiveCamera()->SetParallelProjection(false);
  connect(settings(), SIGNAL(valueChangedFor(QString)), this, SLOT(settingsChangedSlot(QString)));

  mImageLandmarkRep = ImageLandmarkRep::New("ImageLandmarkRep_"+index);
  mImageLandmarkRep->setGraphicsSize(settings()->value("View3D/sphereRadius").toDouble());
  mImageLandmarkRep->setLabelSize(settings()->value("View3D/labelSize").toDouble());

  mPatientLandmarkRep = PatientLandmarkRep::New("PatientLandmarkRep_"+index);
  mPatientLandmarkRep->setGraphicsSize(settings()->value("View3D/sphereRadius").toDouble());
  mPatientLandmarkRep->setLabelSize(settings()->value("View3D/labelSize").toDouble());

  mProbeRep = ssc::ProbeRep::New("ProbeRep_"+index, "ProbeRep_"+index);

  connect(mProbeRep.get(), SIGNAL(pointPicked(ssc::Vector3D)),this, SLOT(probeRepPointPickedSlot(ssc::Vector3D)));
  mProbeRep->setSphereRadius(settings()->value("View3D/sphereRadius").toDouble());
  mProbeRep->setEnabled(false);
  mView->addRep(mProbeRep);
  connect(ssc::toolManager(), SIGNAL(dominantToolChanged(const QString&)), this, SLOT(dominantToolChangedSlot()));
  this->dominantToolChangedSlot();

  // plane type text rep
  mPlaneTypeText = ssc::DisplayTextRep::New("planeTypeRep_"+mView->getName(), "");
  mPlaneTypeText->addText(ssc::Vector3D(0,1,0), "3D", ssc::Vector3D(0.98, 0.02, 0.0));
  mView->addRep(mPlaneTypeText);

  //data name text rep
  mDataNameText = ssc::DisplayTextRep::New("dataNameText_"+mView->getName(), "");
  mDataNameText->addText(ssc::Vector3D(0,1,0), "not initialized", ssc::Vector3D(0.02, 0.02, 0.0));
  mView->addRep(mDataNameText);

  connect(ssc::toolManager(), SIGNAL(configured()), this, SLOT(toolsAvailableSlot()));
  connect(ssc::toolManager(), SIGNAL(initialized()), this, SLOT(toolsAvailableSlot()));
  connect(ssc::dataManager(), SIGNAL(activeImageChanged(const QString&)), this, SLOT(activeImageChangedSlot()));
  this->toolsAvailableSlot();

  mAnnotationMarker = ssc::OrientationAnnotation3DRep::New("annotation_"+mView->getName(), "");
  mAnnotationMarker->setMarkerFilename(DataLocations::getRootConfigPath()+"/models/"+settings()->value("View3D/annotationModel").toString());
  mAnnotationMarker->setSize(settings()->value("View3D/annotationModelSize").toDouble());

  mView->addRep(mAnnotationMarker);
//  mAnnotationMarker->setVisible(settings()->value("View3D/showOrientationAnnotation").toBool());

  //Stereo
//  mView->getRenderWindow()->StereoCapableWindowOn(); // Moved to cxView3D
  connect(settings(), SIGNAL(valueChangedFor(QString)), this, SLOT(globalConfigurationFileChangedSlot(QString)));
  //Init 3D stereo from settings
  this->setStereoType(settings()->value("View3D/stereoType").toInt());
  this->setStereoEyeAngle(settings()->value("View3D/eyeAngle").toDouble());

  this->updateView();
}

ViewWrapper3D::~ViewWrapper3D()
{
  if (mView)
  {
    mView->removeReps();
  }
}

void ViewWrapper3D::settingsChangedSlot(QString key)
{
	if (key=="backgroundColor")
	{
	  QColor background = settings()->value("backgroundColor").value<QColor>();
	  mView->setBackgoundColor(background);
	}
	if (key=="useGPUVolumeRayCastMapper" || "maxRenderSize")
	{
		// reload volumes from cache
	  std::vector<ssc::ImagePtr> images = mViewGroup->getImages();
	  for (unsigned i=0; i<images.size(); ++i)
	  {
	  	this->dataRemoved(images[i]->getUid());
	  	this->dataAdded(images[i]);
	  }
	}
	if (key=="View/showDataText")
	{
	  this->updateView();
	}
	if (key=="View3D/annotationModelSize" || key=="View3D/annotationModel")
	{
	  mAnnotationMarker->setMarkerFilename(DataLocations::getRootConfigPath()+"/models/"+settings()->value("View3D/annotationModel").toString());
    mAnnotationMarker->setSize(settings()->value("View3D/annotationModelSize").toDouble());
	}
  if (key=="View3D/sphereRadius" || key=="View3D/labelSize" || key=="View/showLabels")
  {
    for (RepMap::iterator iter=mDataReps.begin(); iter!=mDataReps.end(); ++iter)
    {
      this->readDataRepSettings(iter->second);
    }

    mImageLandmarkRep->setGraphicsSize(settings()->value("View3D/sphereRadius").toDouble());
    mImageLandmarkRep->setLabelSize(settings()->value("View3D/labelSize").toDouble());
    mPatientLandmarkRep->setGraphicsSize(settings()->value("View3D/sphereRadius").toDouble());
    mPatientLandmarkRep->setLabelSize(settings()->value("View3D/labelSize").toDouble());

  }
}


void ViewWrapper3D::probeRepPointPickedSlot(ssc::Vector3D p_r)
{
	ssc::Transform3D rMpr = *ssc::toolManager()->get_rMpr();
//  ssc::Vector3D p_r(x,y,z); // assume p is in r ...?
  ssc::Vector3D p_pr = rMpr.inv().coord(p_r);

  // set the picked point as offset tip
  ssc::ManualToolPtr tool = ToolManager::getInstance()->getManualTool();
  ssc::Vector3D offset = tool->get_prMt().vector(ssc::Vector3D(0,0,tool->getTooltipOffset()));
  p_pr -= offset;
  p_r = rMpr.coord(p_pr);

//	std::cout << "ViewWrapper3D::probeRepPointPickedSlot " << p_r << p_r<< std::endl;

  // TODO set center here will not do: must handle
  ssc::dataManager()->setCenter(p_r);
  tool->set_prMt(ssc::createTransformTranslate(p_pr));
}

void ViewWrapper3D::appendToContextMenu(QMenu& contextMenu)
{
  QAction* slicePlanesAction = new QAction("Show Slice Planes", &contextMenu);
  slicePlanesAction->setCheckable(true);
  slicePlanesAction->setChecked(mSlicePlanes3DRep->getProxy()->getVisible());
  connect(slicePlanesAction, SIGNAL(triggered(bool)), this, SLOT(showSlicePlanesActionSlot(bool)));

  QAction* fillSlicePlanesAction = new QAction("Fill Slice Planes", &contextMenu);
  fillSlicePlanesAction->setCheckable(true);
  fillSlicePlanesAction->setEnabled(mSlicePlanes3DRep->getProxy()->getVisible());
  fillSlicePlanesAction->setChecked(mSlicePlanes3DRep->getProxy()->getDrawPlanes());
  connect(fillSlicePlanesAction, SIGNAL(triggered(bool)), this, SLOT(fillSlicePlanesActionSlot(bool)));

  QAction* resetCameraAction = new QAction("Reset Camera (r)", &contextMenu);
  connect(resetCameraAction, SIGNAL(triggered()), this, SLOT(resetCameraActionSlot()));

  QAction* centerImageAction = new QAction("Center to image", &contextMenu);
  connect(centerImageAction, SIGNAL(triggered()), this, SLOT(centerImageActionSlot()));

  QAction* centerToolAction = new QAction("Center to tool", &contextMenu);
  connect(centerToolAction, SIGNAL(triggered()), this, SLOT(centerToolActionSlot()));

  QAction* showAxesAction = new QAction("Show Coordinate Axes", &contextMenu);
  showAxesAction->setCheckable(true);
  showAxesAction->setChecked(mShowAxes);
  connect(showAxesAction, SIGNAL(triggered(bool)), this, SLOT(showAxesActionSlot(bool)));

  QAction* showManualTool = new QAction("Show Manual Tool", &contextMenu);
  showManualTool->setCheckable(true);
  showManualTool->setChecked(ToolManager::getInstance()->getManualTool()->getVisible());
  connect(showManualTool, SIGNAL(triggered(bool)), this, SLOT(showManualToolSlot(bool)));

  QAction* showOrientation = new QAction("Show Orientation", &contextMenu);
  showOrientation->setCheckable(true);
  showOrientation->setChecked(mAnnotationMarker->getVisible());
  connect(showOrientation, SIGNAL(triggered(bool)), this, SLOT(showOrientationSlot(bool)));

  QAction* showToolPath = new QAction("Show Tool Path", &contextMenu);
  showToolPath->setCheckable(true);
  ssc::ToolRep3DPtr activeRep3D = RepManager::findFirstRep<ssc::ToolRep3D>(mView->getReps(), ssc::toolManager()->getDominantTool());
  showToolPath->setChecked(activeRep3D->getTracer()->isRunning());
  connect(showToolPath, SIGNAL(triggered(bool)), this, SLOT(showToolPathSlot(bool)));


  QAction* showRefTool = new QAction("Show Reference Tool", &contextMenu);
  showRefTool->setDisabled(true);
  showRefTool->setCheckable(true);
  ssc::ToolPtr refTool = ToolManager::getInstance()->getReferenceTool();
  if(refTool)
  {
    showRefTool->setText("Show "+refTool->getName());
    showRefTool->setEnabled(true);
    showRefTool->setChecked(RepManager::findFirstRep<ssc::ToolRep3D>(mView->getReps(), refTool));
    connect(showRefTool, SIGNAL(toggled(bool)), this, SLOT(showRefToolSlot(bool)));
  }

  contextMenu.addSeparator();
  contextMenu.addAction(resetCameraAction);
  contextMenu.addAction(centerImageAction);
  contextMenu.addAction(centerToolAction);
  contextMenu.addAction(showAxesAction);
  contextMenu.addAction(showOrientation);
  contextMenu.addSeparator();
  contextMenu.addAction(showManualTool);
  contextMenu.addAction(showRefTool);
  contextMenu.addAction(showToolPath);
  contextMenu.addSeparator();
  contextMenu.addAction(slicePlanesAction);
  contextMenu.addAction(fillSlicePlanesAction);
}

void ViewWrapper3D::setViewGroup(ViewGroupDataPtr group)
{
  ViewWrapper::setViewGroup(group);

  connect(group.get(), SIGNAL(initialized()), this, SLOT(resetCameraActionSlot()));
  connect(group.get(), SIGNAL(optionsChanged()), this, SLOT(optionChangedSlot()));
  mView->getRenderer()->SetActiveCamera(mViewGroup->getCamera3D()->getCamera());
  // Set eye angle after camera change. Maybe create a cameraChangedSlot instead
  this->setStereoEyeAngle(settings()->value("View3D/eyeAngle").toDouble());
  this->optionChangedSlot();

}

void ViewWrapper3D::showToolPathSlot(bool checked)
{
  ssc::ToolRep3DPtr activeRep3D = RepManager::findFirstRep<ssc::ToolRep3D>(mView->getReps(), ssc::toolManager()->getDominantTool());
  if (activeRep3D->getTracer()->isRunning())
  {
    activeRep3D->getTracer()->stop();
    activeRep3D->getTracer()->clear();
  }
  else
  {
    activeRep3D->getTracer()->start();
  }

  settings()->setValue("showToolPath", checked);
}


void ViewWrapper3D::showAxesActionSlot(bool checked)
{
	if (mShowAxes==checked)
		return;

	mShowAxes = checked;

	if (mShowAxes)
	{
		if (!mRefSpaceAxisRep)
		{
			  mRefSpaceAxisRep = ssc::AxesRep::New("refspace_axis");
			  mRefSpaceAxisRep->setCaption("ref", ssc::Vector3D(1,0,0));
			  mRefSpaceAxisRep->setFontSize(0.03);
			  mRefSpaceAxisRep->setAxisLength(0.12);

			  mView->addRep(mRefSpaceAxisRep);
		}

	  std::vector<ssc::DataPtr> data = mViewGroup->getData();
	  for (unsigned i=0; i<data.size(); ++i)
	  {
	    ssc::AxesRepPtr rep = ssc::AxesRep::New(data[i]->getName()+"_axis");
      rep->setCaption(data[i]->getName(), ssc::Vector3D(1,0,0));
      rep->setFontSize(0.03);
      rep->setAxisLength(0.08);
      rep->setTransform(data[i]->get_rMd());
      mDataSpaceAxisRep[data[i]->getUid()] = rep;
      mView->addRep(rep);
	  }


		ssc::ToolManager::ToolMapPtr tools = ssc::toolManager()->getTools();
		ssc::ToolManager::ToolMapPtr::value_type::iterator iter;
		for (iter=tools->begin(); iter!=tools->end(); ++iter)
		{
			mToolAxis[iter->first].reset(new ToolAxisConnector(iter->second));
			mToolAxis[iter->first].reset(new ToolAxisConnector(iter->second));
			mView->addRep(mToolAxis[iter->first]->getAxis_t());
			mView->addRep(mToolAxis[iter->first]->getAxis_s());
		}
	}
	else
	{
		mView->removeRep(mRefSpaceAxisRep);
		mRefSpaceAxisRep.reset();

    for (std::map<QString, ssc::AxesRepPtr>::iterator iter=mDataSpaceAxisRep.begin(); iter!=mDataSpaceAxisRep.end(); ++iter)
    {
      mView->removeRep(iter->second);
    }
    mDataSpaceAxisRep.clear();

    std::map<QString, ToolAxisConnectorPtr>::iterator iter;
		for (iter=mToolAxis.begin(); iter!=mToolAxis.end(); ++iter)
		{
			mView->removeRep(iter->second->getAxis_t());
			mView->removeRep(iter->second->getAxis_s());
		}
		mToolAxis.clear();
	}
}

void ViewWrapper3D::showManualToolSlot(bool visible)
{
  settings()->setValue("showManualTool", visible);
  ToolManager::getInstance()->getManualTool()->setVisible(visible);
}

void ViewWrapper3D::showOrientationSlot(bool visible)
{
  settings()->setValue("View/showOrientationAnnotation", visible);
  this->updateView();
}


void ViewWrapper3D::resetCameraActionSlot()
{
  mView->getRenderer()->ResetCamera();
  //Update eye angle after camera is reset
  this->setStereoEyeAngle(settings()->value("View3D/eyeAngle").toDouble());
}

void ViewWrapper3D::centerImageActionSlot()
{
  if (ssc::dataManager()->getActiveImage())
    Navigation().centerToData(ssc::dataManager()->getActiveImage());
  else
    Navigation().centerToView(mViewGroup->getData());
}

void ViewWrapper3D::centerToolActionSlot()
{
  Navigation().centerToTooltip();
}

void ViewWrapper3D::showSlicePlanesActionSlot(bool checked)
{
  mSlicePlanes3DRep->getProxy()->setVisible(checked);
  settings()->setValue("showSlicePlanes", checked);
}
void ViewWrapper3D::fillSlicePlanesActionSlot(bool checked)
{
  mSlicePlanes3DRep->getProxy()->setDrawPlanes(checked);
}

void ViewWrapper3D::dataAdded(ssc::DataPtr data)
{
  if (!mDataReps.count(data->getUid()))
  {
    ssc::RepPtr rep = this->createDataRep3D(data);
    if (!rep)
      return;
    mDataReps[data->getUid()] = rep;
    mView->addRep(rep);
  }

  this->activeImageChangedSlot();
  this->updateView();
}

void ViewWrapper3D::dataRemoved(const QString& uid)
{
  if (!mDataReps.count(uid))
    return;

  mView->removeRep(mDataReps[uid]);
  mDataReps.erase(uid);

  this->activeImageChangedSlot();
  this->updateView();
}

/**Construct a 3D standard rep for a given data.
 *
 */
ssc::RepPtr ViewWrapper3D::createDataRep3D(ssc::DataPtr data)
{
  if (boost::shared_dynamic_cast<ssc::Image>(data))
  {
    ssc::VolumetricRepPtr rep = RepManager::getInstance()->getVolumetricRep(boost::shared_dynamic_cast<ssc::Image>(data));
    return rep;
  }
  else if (boost::shared_dynamic_cast<ssc::Mesh>(data))
  {
    ssc::GeometricRepPtr rep = ssc::GeometricRep::New(data->getUid()+"_geom3D_rep");
    rep->setMesh(boost::shared_dynamic_cast<ssc::Mesh>(data));
    return rep;
  }
  else if (boost::shared_dynamic_cast<PointMetric>(data))
  {
    PointMetricRepPtr rep = PointMetricRep::New(data->getUid()+"_3D_rep");
    this->readDataRepSettings(rep);
    rep->setPointMetric(boost::shared_dynamic_cast<PointMetric>(data));
    return rep;
  }
  else if (boost::shared_dynamic_cast<DistanceMetric>(data))
  {
  	DistanceMetricRepPtr rep = DistanceMetricRep::New(data->getUid()+"_3D_rep");
    this->readDataRepSettings(rep);
    rep->setDistanceMetric(boost::shared_dynamic_cast<DistanceMetric>(data));
    return rep;
  }
  else if (boost::shared_dynamic_cast<AngleMetric>(data))
  {
    AngleMetricRepPtr rep = AngleMetricRep::New(data->getUid()+"_3D_rep");
    this->readDataRepSettings(rep);
    rep->setMetric(boost::shared_dynamic_cast<AngleMetric>(data));
    return rep;
  }
  else if (boost::shared_dynamic_cast<PlaneMetric>(data))
  {
    PlaneMetricRepPtr rep = PlaneMetricRep::New(data->getUid()+"_3D_rep");
    this->readDataRepSettings(rep);
    rep->setMetric(boost::shared_dynamic_cast<PlaneMetric>(data));
    return rep;
  }

  return ssc::RepPtr();
}

/**helper. Read settings common for all data metric reps.
 *
 */
void ViewWrapper3D::readDataRepSettings(ssc::RepPtr rep)
{
  cx::DataMetricRepPtr val = boost::shared_dynamic_cast<DataMetricRep>(rep);
  if (!val)
    return;

  val->setGraphicsSize(settings()->value("View3D/sphereRadius").toDouble());
  val->setShowLabel(settings()->value("View/showLabels").toBool());
  val->setLabelSize(settings()->value("View3D/labelSize").toDouble());
}


void ViewWrapper3D::updateView()
{
  QString text;
  bool show = settings()->value("View/showDataText").value<bool>();

  if (show)
  	text = this->getAllDataNames().join("\n");
  mDataNameText->setText(0, text);
  mDataNameText->setFontSize(std::max(12, 22-2*text.size()));

  mAnnotationMarker->setVisible(settings()->value("View/showOrientationAnnotation").value<bool>());
}

void ViewWrapper3D::activeImageChangedSlot()
{
  ssc::ImagePtr image = ssc::dataManager()->getActiveImage();

  // only show landmarks belonging to image visible in this view:
  std::vector<ssc::ImagePtr> images = mViewGroup->getImages();
  if (!std::count(images.begin(), images.end(), image))
    image.reset();

  mProbeRep->setImage(image);
  mImageLandmarkRep->setImage(image);
  mPatientLandmarkRep->setImage(image);
}

void ViewWrapper3D::showRefToolSlot(bool checked)
{
  ssc::ToolPtr refTool = ssc::toolManager()->getReferenceTool();
  if(!refTool)
    return;
  ssc::ToolRep3DPtr refRep = RepManager::findFirstRep<ssc::ToolRep3D>(mView->getReps(), refTool);
  if(!refRep)
  {
    refRep = ssc::ToolRep3D::New(refTool->getUid()+"_rep3d_"+this->mView->getUid());
    refRep->setTool(refTool);
  }

  if(checked) //should show
    mView->addRep(refRep);
  else//should not show
    mView->removeRep(refRep);
}

ssc::View* ViewWrapper3D::getView()
{
  return mView;
}

void ViewWrapper3D::dominantToolChangedSlot()
{
  ssc::ToolPtr dominantTool = ssc::toolManager()->getDominantTool();
  mProbeRep->setTool(dominantTool);
}


void ViewWrapper3D::toolsAvailableSlot()
{
  ssc::ToolManager::ToolMapPtr tools = ssc::toolManager()->getTools();
  ssc::ToolManager::ToolMapPtr::value_type::iterator iter;
  for (iter=tools->begin(); iter!=tools->end(); ++iter)
  {
    ssc::ToolPtr tool = iter->second;
    if(tool->getType() == ssc::Tool::TOOL_REFERENCE)
      continue;

    ssc::ToolRep3DPtr toolRep = RepManager::findFirstRep<ssc::ToolRep3D>(mView->getReps(), tool);
    if(!toolRep)
    {
      toolRep = ssc::ToolRep3D::New(tool->getUid()+"_rep3d_"+this->mView->getUid());
      if (settings()->value("showToolPath").toBool())
        toolRep->getTracer()->start();
    }

    toolRep->setSphereRadius(settings()->value("View3D/sphereRadius").toDouble()*2/3); // use fraction of set size
    toolRep->setSphereRadiusInNormalizedViewport(true);

    toolRep->setTool(tool);
    toolRep->setOffsetPointVisibleAtZeroOffset(true);
    mView->addRep(toolRep);
  }
}

void ViewWrapper3D::optionChangedSlot()
{
  ViewGroupData::Options options = mViewGroup->getOptions();


  this->showLandmarks(options.mShowLandmarks);
  this->showPointPickerProbe(options.mShowPointPickerProbe);
}

void ViewWrapper3D::showLandmarks(bool on)
{
  if (mImageLandmarkRep->isConnectedToView(mView) == on)
    return;

  if (on)
  {
    mView->addRep(mPatientLandmarkRep);
    //mView->addRep(mImageLandmarkRep);
  }
  else
  {
    mView->removeRep(mPatientLandmarkRep);
    //mView->removeRep(mImageLandmarkRep);
  }
}

void ViewWrapper3D::showPointPickerProbe(bool on)
{
  mProbeRep->setEnabled(on);
}

void ViewWrapper3D::setSlicePlanesProxy(ssc::SlicePlanesProxyPtr proxy)
{
  mSlicePlanes3DRep = ssc::SlicePlanes3DRep::New("uid");
  mSlicePlanes3DRep->setProxy(proxy);
  bool show = settings()->value("showSlicePlanes").toBool();
  mSlicePlanes3DRep->getProxy()->setVisible(show); // init with default value

  mView->addRep(mSlicePlanes3DRep);
}

void ViewWrapper3D::setStereoType(int /*STEREOTYPE*/ type)
{
  switch(type) //STEREOTYPE
      {
      case stFRAME_SEQUENTIAL:
        mView->getRenderWindow()->SetStereoTypeToCrystalEyes();
        break;
      case stINTERLACED:
        mView->getRenderWindow()->SetStereoTypeToInterlaced();
        break;
      case stDRESDEN:
        mView->getRenderWindow()->SetStereoTypeToDresden();
        break;
      case stRED_BLUE:
        mView->getRenderWindow()->SetStereoTypeToRedBlue();
        break;
      }
}

void ViewWrapper3D::globalConfigurationFileChangedSlot(QString key)
{
  if(key == "View3D/stereoType")
  {
    this->setStereoType(settings()->value("View3D/stereoType").toInt());
  }
  else if(key == "View3D/eyeAngle")
  {
    this->setStereoEyeAngle(settings()->value("View3D/eyeAngle").toDouble());
  }
}

void ViewWrapper3D::setStereoEyeAngle(double angle)
{
  mView->getRenderer()->GetActiveCamera()->SetEyeAngle(angle);
}

//------------------------------------------------------------------------------
}
