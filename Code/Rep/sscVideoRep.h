/*
 * sscRT2DRep.h
 *
 *  Created on: Oct 31, 2010
 *      Author: christiana
 */

#ifndef SSCRTSTREAMREP_H_
#define SSCRTSTREAMREP_H_

#include "sscRepImpl.h"
#include "sscVideoSource.h"
#include "sscVtkHelperClasses.h"
#include "sscForwardDeclarations.h"
#include "sscProbeSector.h"

typedef vtkSmartPointer<class vtkPlaneSource> vtkPlaneSourcePtr;
typedef vtkSmartPointer<class vtkTexture> vtkTexturePtr;
typedef vtkSmartPointer<class vtkTransformTextureCoords> vtkTransformTextureCoordsPtr;
typedef vtkSmartPointer<class vtkDataSetMapper> vtkDataSetMapperPtr;
typedef vtkSmartPointer<class vtkActor> vtkActorPtr;
typedef vtkSmartPointer<class vtkRenderer> vtkRendererPtr;
typedef vtkSmartPointer<class vtkTextureMapToPlane> vtkTextureMapToPlanePtr;
typedef vtkSmartPointer<class vtkImageMask> vtkImageMaskPtr;

class UltrasoundSectorSource;

namespace ssc
{

typedef boost::shared_ptr<class VideoGraphics> VideoGraphicsPtr;

class VideoGraphics : public QObject
{
  Q_OBJECT
public:
  VideoGraphics(bool useMaskFilter=false);
  virtual ~VideoGraphics();

  void setRealtimeStream(VideoSourcePtr data);
  void setTool(ToolPtr tool);
  ToolPtr getTool();
  ssc::ProbeSector getProbeData();
  void setClipToSector(bool on);
  void setShowInToolSpace(bool on);
  vtkActorPtr getActor();

signals:
  void newData();

private slots:
  void newDataSlot();
  void receiveTransforms(Transform3D matrix, double timestamp);
  void receiveVisible(bool visible);
  void probeSectorChanged();
  void checkDataIntegrity();

private:
  void setLookupTable();
  void clipToSectorChanged();

  bool mUseMask;
  bool mClipSector;

  bool mShowInToolSpace;
  ToolPtr mTool;
  ssc::ProbeSector mProbeData;
  VideoSourcePtr mData;
  vtkImageChangeInformationPtr mDataRedirecter;
  vtkActorPtr mPlaneActor;
  vtkPlaneSourcePtr mPlaneSource;
  vtkTexturePtr mTexture;
  UltrasoundSectorSource* mUSSource;
  vtkDataSetMapperPtr mDataSetMapper;
  vtkTransformTextureCoordsPtr mTransformTextureCoords;
  vtkTextureMapToPlanePtr mTextureMapToPlane;

  vtkImageThresholdPtr mMapZeroToOne;
  vtkImageMaskPtr mMaskFilter;
};


//typedef boost::shared_ptr<class RealTimeStreamRep> RealTimeStreamRepPtr;
//
//class RealTimeStreamRep : public ssc::RepImpl
//{
//  Q_OBJECT
//public:
//  RealTimeStreamRep(const QString& uid, const QString& name="");
//  virtual ~RealTimeStreamRep();
//  virtual QString getType() const { return "ssc::RealTimeStreamRep"; }
//  void setRealtimeStream(VideoSourcePtr data);
//  void setTool(ToolPtr tool);
//
//protected:
//  virtual void addRepActorsToViewRenderer(ssc::View* view);
//  virtual void removeRepActorsFromViewRenderer(ssc::View* view);
//private slots:
//
//private:
//  VideoGraphicsPtr mRTGraphics;
//  vtkRendererPtr mRenderer;
//  View* mView;
//};

typedef boost::shared_ptr<class VideoFixedPlaneRep> VideoFixedPlaneRepPtr;

/**A rep visualizing a RT stream directly into the view plane.
 * It does not follow the tool, but controls the camera in order to
 * fill the entire view.
 */
class VideoFixedPlaneRep : public ssc::RepImpl
{
  Q_OBJECT
public:
  VideoFixedPlaneRep(const QString& uid, const QString& name="");
  virtual ~VideoFixedPlaneRep();
  virtual QString getType() const { return "ssc::RealTimeStreamFixedPlaneRep"; }
  void setRealtimeStream(VideoSourcePtr data);
  void setTool(ToolPtr tool);

  void setShowSector(bool on);
  bool getShowSector() const;

protected:
  virtual void addRepActorsToViewRenderer(ssc::View* view);
  virtual void removeRepActorsFromViewRenderer(ssc::View* view);
private slots:
  void newDataSlot();

private:
  void setCamera();
  void updateSector();

  VideoGraphicsPtr mRTGraphics;
  bool mShowSector;

  ToolPtr mTool;
  ssc::ProbeSector mProbeData;
  VideoSourcePtr mData;

  ssc::TextDisplayPtr mStatusText;
  ssc::TextDisplayPtr mInfoText;

  vtkPolyDataMapperPtr mProbeSectorPolyDataMapper;
  vtkActorPtr mProbeSectorActor;

  vtkRendererPtr mRenderer;
  View* mView;
};


} // namespace ssc

#endif /* SSCRTSTREAMREP_H_ */
