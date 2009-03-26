#ifndef SSCSURFACEREP_H_
#define SSCSURFACEREP_H_

#include <vtkSmartPointer.h>
typedef vtkSmartPointer<class vtkPolyDataMapper> vtkPolyDataMapperPtr;
typedef	vtkSmartPointer<class vtkContourFilter> vtkContourFilterPtr;
typedef vtkSmartPointer<class vtkProperty> vtkPropertyPtr;
typedef	vtkSmartPointer<class vtkActor> vtkActorPtr;

#include "sscRepImpl.h"
#include "sscMesh.h"
#include "sscImage.h"

namespace ssc
{
typedef boost::shared_ptr<class SurfaceRep> SurfaceRepPtr;

class SurfaceRep : public RepImpl
{
  Q_OBJECT
public:
	static SurfaceRepPtr New(const std::string& uid);
	virtual std::string getType() const { return "ssc::SurfaceRep"; }
	virtual ~SurfaceRep();
	void setMesh(MeshPtr mesh);
	void setImage(ImagePtr image);
	//void removeImage(ImagePtr image);
	bool hasMesh(MeshPtr mesh) const;
protected:
	SurfaceRep(const std::string& uid);
	virtual void addRepActorsToViewRenderer(View* view);
	virtual void removeRepActorsFromViewRenderer(View* view);

	vtkPolyDataMapperPtr mMapper;
	vtkContourFilterPtr mContourFilter;
	vtkPropertyPtr mProperty;
	vtkActorPtr mActor;
	
	ImagePtr mImage;
	MeshPtr mMesh;
};
//---------------------------------------------------------
}//end namespace
//---------------------------------------------------------
#endif /*SSCSURFACEREP_H_*/

