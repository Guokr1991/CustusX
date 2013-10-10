// This file is part of CustusX, an Image Guided Therapy Application.
//
// Copyright (C) 2008- SINTEF Technology & Society, Medical Technology
//
// CustusX is fully owned by SINTEF Medical Technology (SMT). CustusX source
// code and binaries can only be used by SMT and those with explicit permission
// from SMT. CustusX shall not be distributed to anyone else.
//
// CustusX is a research tool. It is NOT intended for use or certified for use
// in a normal clinical setting. SMT does not take responsibility for its use
// in any way.
//
// See CustusX_License.txt for more information.


#include "cxImageEnveloper.h"
#include "sscImage.h"
#include "sscVolumeHelpers.h"
#include "vtkImageData.h"
#include "sscRegistrationTransform.h"
#include "sscTypeConversions.h"
#include "sscBoundingBox3D.h"


namespace cx
{
ImageEnveloperPtr ImageEnveloper::create()
{
	return ImageEnveloperPtr(new ImageEnveloper());
}

void ImageEnveloper::setImages(std::vector<ImagePtr> images)
{
	mImages = images;
}

ImagePtr ImageEnveloper::getEnvelopingImage(long maxVoxels)
{
	ImageParameters box = createEnvelopeParametersFromImage(mImages[0]);
	for(unsigned i = 1; i < mImages.size(); ++i)
		box = selectParametersWithSmallestExtent(box, createEnvelopeParametersFromImage(mImages[i]));
//		box = selectParametersWithFewestVoxels(box, createEnvelopeParametersFromImage(mImages[i]));

//	std::cout << "spacing: " << box.mSpacing << std::endl;
//	std::cout << "dims: " << box.mDim << std::endl;

	box = this->reduceToNumberOfVoxels(box, maxVoxels);
	ImagePtr retval = this->createEnvelopeFromParameters(box);

	return retval;
}

ImageParameters ImageEnveloper::reduceToNumberOfVoxels(ImageParameters box, long maxVoxels)
{
	if((box.getNumVoxels() < maxVoxels) || maxVoxels == 0)
		return box;

	double factor = pow( maxVoxels / box.getNumVoxels(), 1/3.0 );
	box.mDim = box.mDim * factor;
	box.mSpacing = box.mSpacing / factor;

	if(box.getNumVoxels() < maxVoxels)
		std::cout << "error" << std::endl;

	return box;
}

ImageParameters ImageEnveloper::createEnvelopeParametersFromImage(ImagePtr img)
{
	ImageParameters retval;

	DoubleBoundingBox3D bb = findEnclosingBoundingBox(mImages, img->get_rMd().inverse());

	retval.mSpacing = this->getMinimumSpacingFromAllImages(img->get_rMd().inverse());
//	retval.mDim = this->getDimFromExtent(bb.range().array(), retval.mSpacing.array());
	retval.setDimFromBounds(bb.range().array());
	retval.mParentVolume = img->getUid();

//	std::cout << "extent: " << bb.range().array() << std::endl;
//	std::cout << "spacing: " << retval.mSpacing.array() << std::endl;

	Vector3D shift = bb.bottomLeft();

	retval.m_rMd = img->get_rMd() * createTransformTranslate(shift);
	return retval;
}

ImageParameters ImageEnveloper::selectParametersWithSmallestExtent(ImageParameters a, ImageParameters b)
{
	if (a.getVolume() <= b.getVolume())
		return a;
	else
		return b;
}

ImageParameters ImageEnveloper::selectParametersWithFewestVoxels(ImageParameters a, ImageParameters b)
{
	if (a.getNumVoxels() <= b.getNumVoxels())
		return a;
	else
		return b;
}

Eigen::Array3d ImageEnveloper::getMinimumSpacingFromAllImages(Transform3D qMr)
{
	Eigen::Array3d retval;
	retval = this->getTransformedSpacing(mImages[0]->getSpacing(), qMr * mImages[0]->get_rMd());
	for (unsigned i = 1; i < mImages.size(); ++i)
	{
		Eigen::Array3d current = this->getTransformedSpacing(mImages[i]->getSpacing(), qMr * mImages[i]->get_rMd());
		retval = retval.min(current);
	}
	return retval;
}

Eigen::Array3d ImageEnveloper::getTransformedSpacing(Eigen::Array3d spacing, Transform3D qMd)
{
	Eigen::Array3d retval;

	//Create spacing vectors in img coord syst (d)
	Vector3D sx = Vector3D(spacing[0], 0, 0);
	Vector3D sy = Vector3D(0, spacing[1], 0);
	Vector3D sz = Vector3D(0, 0, spacing[2]);

	//Transform to q coord syst
	sx = qMd.vector(sx);
	sy = qMd.vector(sy);
	sz = qMd.vector(sz);

	//Find spacing for each axis
	for (unsigned i = 0; i < 3; ++i)
	{
		retval[i] = std::max(sx[i], sy[i]);
		retval[i] = std::max(retval[i], sz[i]);
	}

	return retval;
}

ImagePtr ImageEnveloper::createEnvelopeFromParameters(ImageParameters box)
{
	vtkImageDataPtr imageData = generateVtkImageData(box.mDim, box.mSpacing, 0, 1);
	QString uid = QString("envelope_image_%1").arg(box.mParentVolume);
	ImagePtr retval(new Image(uid, imageData));
	retval->get_rMd_History()->setRegistration(box.m_rMd);
	retval->get_rMd_History()->setParentSpace(box.mParentVolume);
	retval->setAcquisitionTime(QDateTime::currentDateTime());
	retval->setModality("SC");
	return retval;
}

} // namespace cx