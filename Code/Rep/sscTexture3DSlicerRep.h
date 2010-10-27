#ifndef SSCTEXTURE3DSLICER_H_
#define SSCTEXTURE3DSLICER_H_

#include "sscRepImpl.h"
#include <vector>
#include "sscTransform3D.h"
#include "sscBoundingBox3D.h"

#include "vtkForwardDeclarations.h"
#include "sscForwardDeclarations.h"

//---------------------------------------------------------
namespace ssc
{
typedef vtkSmartPointer<class TextureSlicePainter> TextureSlicePainterPtr;
/**
 * \class Texture3DSlicerRep
 *
 * \brief Slice a volume using a SliceProxy.
 *
 * The functionality is equal to SlicerRepSW, but the actual slicing
 * is performed by loading the image into the GPU as a 3D texture and
 * slicing it there, using the GPU.
 *
 * Used by Sonowand.
 *
 * \date Oct 13, 2009
 * \date petterw
 */
class Texture3DSlicerRep: public ssc::RepImpl
{
Q_OBJECT
public:
	static Texture3DSlicerRepPtr New(const QString& uid);
	virtual ~Texture3DSlicerRep();
	virtual QString getType() const
	{
		return "vm::Texture3DSlicerRep";
	}
	virtual void printSelf(std::ostream & os, ssc::Indent indent);
	void setViewportData(const Transform3D& vpMs, const DoubleBoundingBox3D& vp);
	void setImages(std::vector<ssc::ImagePtr> images);
	void setSliceProxy(ssc::SliceProxyPtr slicer);
	void update();
protected:
	Texture3DSlicerRep(const QString& uid);
	virtual void addRepActorsToViewRenderer(ssc::View* view);
	virtual void removeRepActorsFromViewRenderer(ssc::View* view);
	void createGeometryPlane(Vector3D point1_s, Vector3D point2_s, Vector3D origin_s);

private slots:
	void sliceTransformChangedSlot(Transform3D sMr);
	void updateColorAttributeSlot();
private:
	void createCoordinates(int count);
	void updateCoordinates(int index);
	QString getTCoordName(int index);
	void setColorAttributes(int i);
	DoubleBoundingBox3D mBB_s;
	std::vector<ssc::ImagePtr> mImages;
	ssc::SliceProxyPtr mSliceProxy;

	TextureSlicePainterPtr mPainter;
	vtkActorPtr mActor;
	vtkPolyDataPtr mPolyData;
	vtkPlaneSourcePtr mPlaneSource;
	vtkStripperPtr mStripper;
	vtkPainterPolyDataMapperPtr mPainterPolyDatamapper;

	//std::vector<testApp::SlicedImageProxyPtr> mSlices;
};
//---------------------------------------------------------
}//end namespace
//---------------------------------------------------------
#endif /* SSCTEXTURE3DSLICER_H_ */

