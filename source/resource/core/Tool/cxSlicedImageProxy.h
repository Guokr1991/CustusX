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


#ifndef CXSLICEDIMAGEPROXY_H_
#define CXSLICEDIMAGEPROXY_H_

#include "cxResourceExport.h"

#include "cxIndent.h"
#include "cxTransform3D.h"
#include "vtkForwardDeclarations.h"

namespace cx
{
// forward declarations
typedef boost::shared_ptr<class Image> ImagePtr;
typedef boost::shared_ptr<class SliceProxy> SliceProxyPtr;
typedef boost::shared_ptr<class SliceProxyInterface> SliceProxyInterfacePtr;

typedef boost::shared_ptr<class SlicedImageProxy> SlicedImageProxyPtr;
typedef boost::shared_ptr<class ApplyLUTToImage2DProxy> ApplyLUTToImage2DProxyPtr;

/** \brief Helper class for applying sscLUT2D to an image.
 *
 * Input a 2D image and a lut.
 * Run the image through the lut using a vtkImageMapToColors
 * and return the output.
 *
 * For color input images, the lut is still applied, but is applied
 * to each color component (i.e. relative color distribution is not changed)
 *
 * Used internally in SlicedImageProxy .
 *
 * Used by CustusX
 *
 * \ingroup cx_resource_core_tool
 */
class cxResource_EXPORT ApplyLUTToImage2DProxy : public QObject
{
Q_OBJECT
public:
	ApplyLUTToImage2DProxy();
	virtual ~ApplyLUTToImage2DProxy();
	void setInputData(vtkImageDataPtr image, vtkLookupTablePtr lut);
	void setInput(vtkImageAlgorithmPtr input, vtkLookupTablePtr lut);
	vtkImageDataPtr getOutput();
	vtkImageAlgorithmPtr getOutputPort(); ///< output 2D sliced image

private:
	vtkImageDataPtr mDummyImage; ///< need this to fool the vtk pipeline when no image is set
	vtkImageAlgorithmPtr mRedirecter;
};

/**\brief Helper class for slicing an image given a SliceProxy and an image.
 *
 * The image is sliced in software using the slice definition from
 * the SliceProxy
 *
 * Used internally by BlendedSliceRep and SlicerRepSW as the slice engine.
 * 
 * Used by Sonowand 2.1
 * Used by CustusX
 *
 * \ingroup cx_resource_visualization
 */
class cxResource_EXPORT SlicedImageProxy : public QObject
{
Q_OBJECT
public:
	SlicedImageProxy();
	virtual ~SlicedImageProxy();
	void setSliceProxy(SliceProxyInterfacePtr slicer);
	void setImage(ImagePtr image);
	ImagePtr getImage() const;
	void setOutputFormat(Vector3D origin, Eigen::Array3i dim, Vector3D spacing);
	void update();
	vtkImageDataPtr getOutput(); ///< output 2D sliced image
	vtkImageAlgorithmPtr getOutputPort(); ///< output 2D sliced image
	vtkImageDataPtr getOutputWithoutLUT();
	vtkImageAlgorithmPtr getOutputPortWithoutLUT();
	void printSelf(std::ostream & os, Indent indent);

private slots:
	void transformChangedSlot();
	void transferFunctionsChangedSlot();
	void updateRedirecterSlot();

private: 
	ApplyLUTToImage2DProxyPtr mImageWithLUTProxy;

	SliceProxyInterfacePtr mSlicer;
	ImagePtr mImage;

	vtkImageReslicePtr mReslicer;
	vtkMatrix4x4Ptr mMatrixAxes;

	vtkImageChangeInformationPtr mRedirecter;
};

//---------------------------------------------------------
}//end namespace
//---------------------------------------------------------
#endif /*CXSLICEDIMAGEPROXY_H_*/
