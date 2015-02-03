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


#include "cxImage.h"

#include <QDomDocument>
#include <QDir>
#include <vtkImageAccumulate.h>
#include <vtkImageReslice.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkImageResample.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageClip.h>
#include <vtkImageIterator.h>
#include <vtkImageShiftScale.h>
#include "cxImageTF3D.h"
#include "cxBoundingBox3D.h"
#include "cxImageLUT2D.h"
#include "cxRegistrationTransform.h"
#include "cxLandmark.h"

#include "cxLogger.h"
#include "cxTypeConversions.h"
#include "cxUtilHelpers.h"
#include "cxVolumeHelpers.h"
#include "cxImageDefaultTFGenerator.h"
#include "cxDataReaderWriter.h"
#include "cxNullDeleter.h"

#include "cxUnsignedDerivedImage.h"

typedef vtkSmartPointer<vtkImageChangeInformation> vtkImageChangeInformationPtr;

namespace cx
{

Image::ShadingStruct::ShadingStruct()
{
	on = true;
	ambient = 0.2;
	diffuse = 0.9;
	specular = 0.3;
	specularPower = 15.0;
}

double Image::ShadingStruct::loadAttribute(QDomNode dataNode, QString name, double defVal)
{
	QString text = dataNode.toElement().attribute(name);
	bool ok;
	double val = text.toDouble(&ok);
	if (ok)
		return val;
	return defVal;
}

void Image::ShadingStruct::addXml(QDomNode dataNode)
{
	QDomElement elem = dataNode.toElement();
	elem.setAttribute("on", on);
	elem.setAttribute("ambient", ambient);
	elem.setAttribute("diffuse", diffuse);
	elem.setAttribute("specular", specular);
	elem.setAttribute("specularPower", specularPower);
}

void Image::ShadingStruct::parseXml(QDomNode dataNode)
{
	if (dataNode.isNull())
		return;

	on = dataNode.toElement().attribute("on").toInt();
	//	std::cout << "attrib on: " << dataNode.toElement().attribute("on")  << " : " << on << std::endl;
	ambient = loadAttribute(dataNode, "ambient", ambient);
	diffuse = loadAttribute(dataNode, "diffuse", diffuse);
	specular = loadAttribute(dataNode, "specular", specular);
	specularPower = loadAttribute(dataNode, "specularPower", specularPower);
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

ImagePtr Image::create(const QString& uid, const QString& name)
{
	return ImagePtr(new Image(uid, vtkImageDataPtr(), name));
}

Image::~Image()
{
}

Image::Image(const QString& uid, const vtkImageDataPtr& data, const QString& name) :
	Data(uid, name), mBaseImageData(data), mMaxRGBIntensity(-1)
{
	mInitialWindowWidth = -1;
	mInitialWindowLevel = -1;

	mInterpolationType = VTK_LINEAR_INTERPOLATION;
	mUseCropping = false;
	mCroppingBox_d = this->getInitialBoundingBox();
    mModality = "UNKNOWN";

	mImageLookupTable2D.reset();
	mImageTransferFunctions3D.reset();

	this->setAcquisitionTime(QDateTime::currentDateTime());
}

ImagePtr Image::copy()
{
	vtkImageDataPtr baseImageDataCopy;
	if(mBaseImageData)
	{
		baseImageDataCopy = vtkImageDataPtr::New();
		baseImageDataCopy->DeepCopy(mBaseImageData);
	}

	ImagePtr retval = ImagePtr(new Image(mUid, baseImageDataCopy, mName));

	retval->mUnsigned = mUnsigned;
	retval->mModality = mModality;
	retval->mImageType = mImageType;
	retval->mMaxRGBIntensity = mMaxRGBIntensity;
	retval->mInterpolationType = mInterpolationType;
	retval->mImageLookupTable2D = mImageLookupTable2D;
	retval->mImageTransferFunctions3D = mImageTransferFunctions3D;
	retval->mInitialWindowWidth = mInitialWindowWidth;
	retval->mInitialWindowLevel = mInitialWindowLevel;

	//From cx::Data
	retval->mRegistrationStatus = mRegistrationStatus;
	retval->m_rMd_History = m_rMd_History;

	return retval;
}

void Image::intitializeFromParentImage(ImagePtr parentImage)
{
	this->get_rMd_History()->setRegistration(parentImage->get_rMd());
	this->get_rMd_History()->setParentSpace(parentImage->getUid());
	ImageTF3DPtr transferFunctions = parentImage->getTransferFunctions3D()->createCopy();
	ImageLUT2DPtr LUT2D = parentImage->getLookupTable2D()->createCopy();
	this->setLookupTable2D(LUT2D);
	this->setTransferFunctions3D(transferFunctions);
	this->setModality(parentImage->getModality());
	this->setImageType(parentImage->getImageType());
	this->setShading(parentImage->getShading());
	mInitialWindowWidth = parentImage->getInitialWindowWidth();
	mInitialWindowLevel = parentImage->getInitialWindowLevel();
}

DoubleBoundingBox3D Image::getInitialBoundingBox() const
{
	return DoubleBoundingBox3D(-1, -1, -1, -1, -1, -1);
}

ImagePtr Image::getUnsigned(ImagePtr self)
{
	CX_ASSERT(this==self.get());

	if (!mUnsigned)
	{
		// self is unsigned: return self
		if (this->getBaseVtkImageData()->GetScalarTypeMin() >= 0)
			return self;
		else // signed: create unsigned adapter
			mUnsigned = UnsignedDerivedImage::create(self);
	}

	return mUnsigned;
}



void Image::resetTransferFunctions(bool _2D, bool _3D)
{
	if (!mBaseImageData)
	{
		reportWarning("Image has no image data");
		return;
	}

	mBaseImageData->GetScalarRange(); // this line updates some internal vtk value, and (on fedora) removes 4.5s in the second render().
	mMaxRGBIntensity = -1;

	ImageDefaultTFGenerator tfGenerator(ImagePtr(this, null_deleter()));
	if (_3D)
	{
		this->resetTransferFunction(tfGenerator.generate3DTFPreset());
		tfGenerator.resetShading();
	}
	if (_2D)
		this->resetTransferFunction(tfGenerator.generate2DTFPreset());
}

void Image::resetTransferFunction(ImageTF3DPtr imageTransferFunctions3D, ImageLUT2DPtr imageLookupTable2D)
{
	this->blockSignals(true); // avoid emitting two transferFunctionsChanged() for one call.

	this->resetTransferFunction(imageTransferFunctions3D);
	this->resetTransferFunction(imageLookupTable2D);

	this->blockSignals(false);
	emit transferFunctionsChanged();
}

void Image::resetTransferFunction(ImageLUT2DPtr imageLookupTable2D)
{
	if (mImageLookupTable2D)
	{
		disconnect(mImageLookupTable2D.get(), SIGNAL(transferFunctionsChanged()), this,
			SIGNAL(transferFunctionsChanged()));
	}

	mImageLookupTable2D = imageLookupTable2D;

	if (mImageLookupTable2D)
	{
		connect(mImageLookupTable2D.get(), SIGNAL(transferFunctionsChanged()), this, SIGNAL(transferFunctionsChanged()));
	}

	emit transferFunctionsChanged();
}

void Image::resetTransferFunction(ImageTF3DPtr imageTransferFunctions3D)
{
	if (mImageTransferFunctions3D)
	{
		disconnect(mImageTransferFunctions3D.get(), SIGNAL(transferFunctionsChanged()), this,
			SIGNAL(transferFunctionsChanged()));
	}

	mImageTransferFunctions3D = imageTransferFunctions3D;

	if (mImageTransferFunctions3D)
	{
		connect(mImageTransferFunctions3D.get(), SIGNAL(transferFunctionsChanged()), this,
			SIGNAL(transferFunctionsChanged()));
	}

	emit transferFunctionsChanged();
}

void Image::transformChangedSlot()
{
}

void Image::moveThisAndChildrenToThread(QThread* thread)
{
	  // important! move thread affinity to main thread - ensures signals/slots is still called correctly
	  this->moveToThread(thread);
	  this->getTransferFunctions3D()->moveToThread(thread);
	  this->getLookupTable2D()->moveToThread(thread);
	  this->get_rMd_History()->moveToThread(thread);
}

void Image::setVtkImageData(const vtkImageDataPtr& data)
{
	mBaseImageData = data;
	mBaseGrayScaleImageData = NULL;

	this->resetTransferFunctions();
	emit vtkImageDataChanged();
}

vtkImageDataPtr Image::get8bitGrayScaleVtkImageData()
{
	double windowWidth = mImageLookupTable2D->getWindow();
	double windowLevel = mImageLookupTable2D->getLevel();
	return convertImageDataTo8Bit(this->getGrayScaleVtkImageData(), windowWidth, windowLevel);
}

vtkImageDataPtr Image::getGrayScaleVtkImageData()
{
	if (mBaseGrayScaleImageData)
	{
		return mBaseGrayScaleImageData;
	}

	mBaseGrayScaleImageData = convertImageDataToGrayScale(this->getBaseVtkImageData());
	return mBaseGrayScaleImageData;
}

ImageTF3DPtr Image::getTransferFunctions3D()
{
	if(!this->mImageTransferFunctions3D)
		this->resetTransferFunctions(false, true);
	return mImageTransferFunctions3D;
}

void Image::setTransferFunctions3D(ImageTF3DPtr transferFuntion)
{
	this->resetTransferFunction(transferFuntion);
}

ImageLUT2DPtr Image::getLookupTable2D()
{
	if(!mImageLookupTable2D)
		this->resetTransferFunctions(true, false);
	return mImageLookupTable2D;
}

void Image::setLookupTable2D(ImageLUT2DPtr imageLookupTable2D)
{
	this->resetTransferFunction(imageLookupTable2D);
}

vtkImageDataPtr Image::getBaseVtkImageData()
{
	return mBaseImageData;
}

//vtkImageDataPtr Image::getRefVtkImageData()
//{
//	if (!mReferenceImageData) // optimized: don't init it if you don't need it.
//	{
//		// provide a resampled volume for algorithms requiring that (such as PickerRep)
//		mOrientatorMatrix = vtkMatrix4x4Ptr::New();
//		mOrientator = vtkImageReslicePtr::New();
//		mOrientator->SetInput(mBaseImageData);
//		mOrientator->SetInterpolationModeToLinear();
//		mOrientator->SetOutputDimensionality(3);
//		mOrientator->SetResliceAxes(mOrientatorMatrix);
//		mOrientator->AutoCropOutputOn();
//		mReferenceImageData = mOrientator->GetOutput();

//		mReferenceImageData->Update();

//		this->transformChangedSlot(); // update transform
//		std::cout << "Warning: Image::getRefVtkImageData() called. Expensive. Do not use." << std::endl;
//	}

//	return mReferenceImageData;
//}

DoubleBoundingBox3D Image::boundingBox() const
{
//	mBaseImageData->UpdateInformation();
	DoubleBoundingBox3D bounds(mBaseImageData->GetBounds());
	return bounds;
}

Eigen::Array3d Image::getSpacing() const
{
	return Eigen::Array3d(mBaseImageData->GetSpacing());
}

vtkImageAccumulatePtr Image::getHistogram()
{
	if (mHistogramPtr.GetPointer() == NULL)
	{
		mHistogramPtr = vtkImageAccumulatePtr::New();
		mHistogramPtr->SetInputData(this->getGrayScaleVtkImageData());
		mHistogramPtr->IgnoreZeroOn(); // required for Sonowand CT volumes, where data are placed between 31K and 35K.
		// Set up only a 1D histogram for now, so y and z values are set to 0
		mHistogramPtr->SetComponentExtent(0, this->getRange(), 0, 0, 0, 0);
		mHistogramPtr->SetComponentOrigin(this->getMin(), 0, 0);
		mHistogramPtr->SetComponentSpacing(1, 0, 0);
	}
	mHistogramPtr->Update();
	return mHistogramPtr;
}

template<typename scalartype> static int getRGBMax(vtkImageDataPtr image)
{
	int max = 0;
	vtkImageIterator<scalartype> iter(image, image->GetExtent());
	while (!iter.IsAtEnd())
	{
		typename vtkImageIterator<scalartype>::SpanIterator siter = iter.BeginSpan();
		while (siter != iter.EndSpan())
		{
			int value = *siter;
			++siter;
			value += *siter;
			++siter;
			value += *siter;
			++siter;
			if (value > max)
			{
				max = value;
			}
		}
		iter.NextSpan();
	}
	return max/3;
}


int Image::getMax()
{
	// Alternatively create max from histogram
	//IntIntMap::iterator iter = this->getHistogram()->end();
	//iter--;
	//return (*iter).first;
	if (mBaseImageData->GetNumberOfScalarComponents() == 3)
	{
		if (mMaxRGBIntensity != -1)
		{
			return mMaxRGBIntensity;
		}
		QDateTime before = QDateTime::currentDateTime();
		double max = 0.0;
		switch (mBaseImageData->GetScalarType())
		{
		case VTK_UNSIGNED_CHAR:
			max = getRGBMax<unsigned char>(mBaseImageData);
			break;
		case VTK_UNSIGNED_SHORT:
			max = getRGBMax<unsigned short>(mBaseImageData);
			break;
		default:
			CX_LOG_ERROR() << "Unhandled RGB data type in image " << this->getUid();
			break;
		}
		mMaxRGBIntensity = max;
		return (int)mMaxRGBIntensity;
	}
	else
	{
//		return (int) this->getTransferFunctions3D()->getScalarMax();
		return mBaseImageData->GetScalarRange()[1];
	}
}

int Image::getMin()
{
	// Alternatively create min from histogram
	//IntIntMap::iterator iter = this->getHistogram()->begin();
	//return (*iter).first;
	return mBaseImageData->GetScalarRange()[0];
//	return (int) this->getTransferFunctions3D()->getScalarMin();
}

int Image::getRange()
{
	return this->getMax() - this->getMin();
}

int Image::getMaxAlphaValue()
{
	return 255;
}

//LandmarksPtr Image::getLandmarks()
//{
//	return mLandmarks;
//}

void Image::addXml(QDomNode& dataNode)
{
	Data::addXml(dataNode);
	QDomNode imageNode = dataNode;
	QDomDocument doc = dataNode.ownerDocument();

	QDomElement tf3DNode = doc.createElement("transferfunctions");
	this->getTransferFunctions3D()->addXml(tf3DNode);
	imageNode.appendChild(tf3DNode);

	QDomElement lut2DNode = doc.createElement("lookuptable2D");
	this->getLookupTable2D()->addXml(lut2DNode);
	imageNode.appendChild(lut2DNode);

	QDomElement shadingNode = doc.createElement("shading");
	mShading.addXml(shadingNode);
	imageNode.appendChild(shadingNode);

//	QDomElement landmarksNode = doc.createElement("landmarks");
//	mLandmarks->addXml(landmarksNode);
//	imageNode.appendChild(landmarksNode);

	QDomElement cropNode = doc.createElement("crop");
	cropNode.setAttribute("use", mUseCropping);
	cropNode.appendChild(doc.createTextNode(qstring_cast(mCroppingBox_d)));
	imageNode.appendChild(cropNode);

	QDomElement clipNode = doc.createElement("clip");
	for (unsigned i = 0; i < mPersistentClipPlanes.size(); ++i)
	{
		QDomElement planeNode = doc.createElement("plane");
		Vector3D normal(mPersistentClipPlanes[i]->GetNormal());
		Vector3D origin(mPersistentClipPlanes[i]->GetOrigin());
		planeNode.setAttribute("normal", qstring_cast(normal));
		planeNode.setAttribute("origin", qstring_cast(origin));
		clipNode.appendChild(planeNode);
	}
	imageNode.appendChild(clipNode);

	QDomElement modalityNode = doc.createElement("modality");
	modalityNode.appendChild(doc.createTextNode(mModality));
	imageNode.appendChild(modalityNode);

	QDomElement imageTypeNode = doc.createElement("imageType");
	imageTypeNode.appendChild(doc.createTextNode(mImageType));
	imageNode.appendChild(imageTypeNode);

	QDomElement interpolationNode = doc.createElement("vtk_interpolation");
	interpolationNode.setAttribute("type", mInterpolationType);
	imageNode.appendChild(interpolationNode);

	QDomElement initialWindowNode = doc.createElement("initialWindow");
	initialWindowNode.setAttribute("width", mInitialWindowWidth);
	initialWindowNode.setAttribute("level", mInitialWindowLevel);
}

double Image::loadAttribute(QDomNode dataNode, QString name, double defVal)
{
	QString text = dataNode.toElement().attribute(name);
	bool ok;
	double val = text.toDouble(&ok);
	if (ok)
		return val;
	return defVal;
}

bool Image::load(QString path)
{
	ImagePtr self = ImagePtr(this, null_deleter());
	DataReaderWriter().readInto(self, path);
	return this->getBaseVtkImageData()!=0;
}

void Image::parseXml(QDomNode& dataNode)
{
	Data::parseXml(dataNode);

	// image node must be parsed in the data manager to create this Image object
	// Only subnodes are parsed here

	if (dataNode.isNull())
		return;

	//transferefunctions
	QDomNode transferfunctionsNode = dataNode.namedItem("transferfunctions");
	if (!transferfunctionsNode.isNull())
		this->getTransferFunctions3D()->parseXml(transferfunctionsNode);
	else
	{
		std::cout << "Warning: Image::parseXml() found no transferfunctions";
		std::cout << std::endl;
	}

	mInitialWindowWidth = this->loadAttribute(dataNode.namedItem("initialWindow"), "width", -1);
	mInitialWindowLevel = this->loadAttribute(dataNode.namedItem("initialWindow"), "level", -1);

	this->getLookupTable2D()->parseXml(dataNode.namedItem("lookuptable2D"));

	// backward compatibility:
	mShading.on = dataNode.namedItem("shading").toElement().text().toInt();
	//Assign default values if the shading nodes don't exists to allow backward compability
	if (!dataNode.namedItem("shadingAmbient").isNull())
		mShading.ambient = dataNode.namedItem("shadingAmbient").toElement().text().toDouble();
	if (!dataNode.namedItem("shadingDiffuse").isNull())
		mShading.diffuse = dataNode.namedItem("shadingDiffuse").toElement().text().toDouble();
	if (!dataNode.namedItem("shadingSpecular").isNull())
		mShading.specular = dataNode.namedItem("shadingSpecular").toElement().text().toDouble();
	if (!dataNode.namedItem("shadingSpecularPower").isNull())
		mShading.specularPower = dataNode.namedItem("shadingSpecularPower").toElement().text().toDouble();

	// new way:
	mShading.parseXml(dataNode.namedItem("shading"));

//	mLandmarks->parseXml(dataNode.namedItem("landmarks"));

	QDomElement cropNode = dataNode.namedItem("crop").toElement();
	if (!cropNode.isNull())
	{
		mUseCropping = cropNode.attribute("use").toInt();
		mCroppingBox_d = DoubleBoundingBox3D::fromString(cropNode.text());
	}

	QDomElement clipNode = dataNode.namedItem("clip").toElement();
	QDomElement clipPlaneNode = clipNode.firstChildElement("plane");
	for (; !clipPlaneNode.isNull(); clipPlaneNode = clipPlaneNode.nextSiblingElement("plane"))
	{
		Vector3D normal = Vector3D::fromString(clipPlaneNode.attribute("normal"));
		Vector3D origin = Vector3D::fromString(clipPlaneNode.attribute("origin"));
		vtkPlanePtr plane = vtkPlanePtr::New();
		plane->SetNormal(normal.begin());
		plane->SetOrigin(origin.begin());
		mPersistentClipPlanes.push_back(plane);
	}

	mModality = dataNode.namedItem("modality").toElement().text();
	mImageType = dataNode.namedItem("imageType").toElement().text();

	QDomElement interpoationNode = dataNode.namedItem("vtk_interpolation").toElement();
	if (!interpoationNode.isNull())
	{
		mInterpolationType = interpoationNode.attribute("type").toInt();
		emit vtkImageDataChanged();
	}
}

void Image::setInitialWindowLevel(double width, double level)
{
	mInitialWindowWidth = width;
	mInitialWindowLevel = level;
}

void Image::setShadingOn(bool on)
{
	mShading.on = on;
	emit transferFunctionsChanged();
}

bool Image::getShadingOn() const
{
	return mShading.on;
}

void Image::setShadingAmbient(double ambient)
{
	mShading.ambient = ambient;
	emit transferFunctionsChanged();
}

void Image::setShadingDiffuse(double diffuse)
{
	mShading.diffuse = diffuse;
	emit transferFunctionsChanged();
}

void Image::setShadingSpecular(double specular)
{
	mShading.specular = specular;
	emit transferFunctionsChanged();
}

void Image::setShadingSpecularPower(double specularPower)
{
	mShading.specularPower = specularPower;
	emit transferFunctionsChanged();
}

double Image::getShadingAmbient()
{
	return mShading.ambient;
}

double Image::getShadingDiffuse()
{
	return mShading.diffuse;
}

double Image::getShadingSpecular()
{
	return mShading.specular;
}

double Image::getShadingSpecularPower()
{
	return mShading.specularPower;
}

Image::ShadingStruct Image::getShading()
{
	return mShading;
}

void Image::setShading(Image::ShadingStruct shading)
{
	mShading = shading;
	emit transferFunctionsChanged();
}

// methods for defining and storing a cropping box. Image does not use these data, this is up to the mapper
void Image::setCropping(bool on)
{
	if (mUseCropping == on)
		return;

	mUseCropping = on;
	if (similar(mCroppingBox_d, this->getInitialBoundingBox()))
		mCroppingBox_d = this->boundingBox();
	emit cropBoxChanged();
}

bool Image::getCropping() const
{
	return mUseCropping;
}

void Image::setCroppingBox(const DoubleBoundingBox3D& bb_d)
{
	if (similar(mCroppingBox_d, bb_d))
		return;
	mCroppingBox_d = bb_d;
	emit cropBoxChanged();
}

DoubleBoundingBox3D Image::getCroppingBox() const
{
	if (similar(mCroppingBox_d, this->getInitialBoundingBox()))
		return this->boundingBox();
	return mCroppingBox_d;
}

// methods for defining and storing clip planes. Image does not use these data, this is up to the mapper
void Image::addPersistentClipPlane(vtkPlanePtr plane)
{
	if (std::count(mPersistentClipPlanes.begin(), mPersistentClipPlanes.end(), plane))
		return;
	mPersistentClipPlanes.push_back(plane);
	emit clipPlanesChanged();
}

std::vector<vtkPlanePtr> Image::getAllClipPlanes()
{
	std::vector<vtkPlanePtr> retval = mPersistentClipPlanes;
	if (mInteractiveClipPlane)
		retval.push_back(mInteractiveClipPlane);
	return retval;
}

void Image::clearPersistentClipPlanes()
{
	mPersistentClipPlanes.clear();
	emit clipPlanesChanged();
}

void Image::setInteractiveClipPlane(vtkPlanePtr plane)
{
	mInteractiveClipPlane = plane;
	emit clipPlanesChanged();
}

/**Do the following operations on mBaseVtkImageData:
 *  * Reset the origin to zero.
 *  * Reset the extent to have its lower-left corner in zero.
 * The shift introduced by these  two operations are inserted
 * as a translation into the matrix rMd.
 *
 * This operation is needed because Image dont support vtkImageData
 * with a nonzero origin or nonzero extent. These must be removed during creation.
 *
 * Use this method only when you, by using some vtk algorithm, have created a vtkImageData
 * that in nonconform with the Image spec.
 */
void Image::mergevtkSettingsIntosscTransform()
{
	// the internal CustusX format does not handle extents starting at non-zero.
	// Move extent to zero and change rMd.
	Vector3D origin(mBaseImageData->GetOrigin());
	Vector3D spacing(mBaseImageData->GetSpacing());
	IntBoundingBox3D extent(mBaseImageData->GetExtent());
	Vector3D extentShift = multiply_elems(extent.corner(0, 0, 0).cast<double>(), spacing);

	vtkImageChangeInformationPtr info = vtkImageChangeInformationPtr::New();
	info->SetInputData(mBaseImageData);
	info->SetOutputExtentStart(0, 0, 0);
	info->SetOutputOrigin(0, 0, 0);
	info->Update();
	info->UpdateInformation();
	mBaseImageData = info->GetOutput();

	mBaseImageData->ComputeBounds();
//	mBaseImageData->Update();
//	mBaseImageData->UpdateInformation();

	this->get_rMd_History()->setRegistration(this->get_rMd() * createTransformTranslate(origin + extentShift));

	emit vtkImageDataChanged();
	emit clipPlanesChanged();
	emit cropBoxChanged();
}

QString Image::getModality() const
{
	return mModality;
}

void Image::setModality(const QString& val)
{
	mModality = val;
	emit propertiesChanged();
}

QString Image::getImageType() const
{
	return mImageType;
}

void Image::setImageType(const QString& val)
{
	mImageType = val;
	emit propertiesChanged();
}

vtkImageDataPtr Image::createDummyImageData(int axisSize, int maxVoxelValue)
{
	int size = axisSize - 1;//Modify axis size as extent starts with 0, not 1
	vtkImageDataPtr dummyImageData = vtkImageDataPtr::New();
	dummyImageData->SetExtent(0, size, 0, size, 0, size);
	dummyImageData->SetSpacing(1, 1, 1);
	//dummyImageData->SetScalarTypeToUnsignedShort();
//	dummyImageData->SetScalarTypeToUnsignedChar();
//	dummyImageData->SetNumberOfScalarComponents(1);
//	dummyImageData->AllocateScalars();
	dummyImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	unsigned char* dataPtr = static_cast<unsigned char*> (dummyImageData->GetScalarPointer());

	//Init voxel colors
	int minVoxelValue = 0;
	int numVoxels = axisSize*axisSize*axisSize;
	for (int i = 0; i < numVoxels; ++i)
	{
		int voxelValue = minVoxelValue + i;
		if (i == numVoxels)
			dataPtr[i] = maxVoxelValue;
		else if (voxelValue < maxVoxelValue)
			dataPtr[i] = voxelValue;
		else
			dataPtr[i] = maxVoxelValue;
	}
	setDeepModified(dummyImageData);
	return dummyImageData;
}

void Image::setInterpolationTypeToNearest()
{
	this->setInterpolationType(VTK_NEAREST_INTERPOLATION);
}
void Image::setInterpolationTypeToLinear()
{
	this->setInterpolationType(VTK_LINEAR_INTERPOLATION);
}
void Image::setInterpolationType(int val)
{
	mInterpolationType = val;
	emit vtkImageDataChanged();
}
int Image::getInterpolationType() const
{
	return mInterpolationType;
}

vtkImageDataPtr Image::resample(long maxVoxels)
{
	// also use grayscale as vtk is incapable of rendering 3component color.
	vtkImageDataPtr retval = this->getGrayScaleVtkImageData();

	double factor = computeResampleFactor(maxVoxels);

	if (fabs(1.0-factor)>0.01) // resampling
	{
		vtkImageResamplePtr resampler = vtkImageResamplePtr::New();
		resampler->SetInterpolationModeToLinear();
		resampler->SetAxisMagnificationFactor(0, factor);
		resampler->SetAxisMagnificationFactor(1, factor);
		resampler->SetAxisMagnificationFactor(2, factor);
		resampler->SetInputData(retval);
//		resampler->GetOutput()->Update();
		resampler->Update();
		resampler->GetOutput()->GetScalarRange();
		retval = resampler->GetOutput();

		long voxelsDown = retval->GetNumberOfPoints();
		long voxelsOrig = this->getBaseVtkImageData()->GetNumberOfPoints();
//		report("Created downsampled volume in Image: "
//									 + this->getName()
//									 + " below " + qstring_cast(voxelsDown/1000/1000) + "M. "
//									 + "Ratio: " + QString::number(factor, 'g', 2) + ", "
//									 + "Original size: " + qstring_cast(voxelsOrig/1000/1000) + "M.");
	}
	return retval;
}

double Image::computeResampleFactor(long maxVoxels)
{
	if (maxVoxels==0)
		return 1.0;

	long voxels = this->getBaseVtkImageData()->GetNumberOfPoints();
	double factor = (double)maxVoxels/(double)voxels;
	factor = pow(factor, 1.0/3.0);
	// cubic function leads to trouble for 138M-volume - must downsample to as low as 5-10 Mv in order to succeed on Mac.

	if (factor<0.99)
	{
		return factor;
	}
	return 1.0;
}

void Image::save(const QString& basePath)
{
	QString filename = basePath + "/Images/" + this->getUid() + ".mhd";
	this->setFilename(QDir(basePath).relativeFilePath(filename));

	MetaImageReader().saveImage(ImagePtr(this), filename);
}

} // namespace cx
