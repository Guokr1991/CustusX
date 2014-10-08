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

#ifndef CX_FORWARDDECLARARATIONS_H_
#define CX_FORWARDDECLARARATIONS_H_

#include "cxPrecompiledHeader.h"

#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <QPointer>
#include "vtkForwardDeclarations.h"

/**\file cxForwardDeclarations.h
 *
 * Include this file when the types and not the
 * full definitions of the ssc is needed.
 */

namespace cx {

typedef boost::shared_ptr<class DataAdapter> DataAdapterPtr;

class View;
class ViewItem;

// KEEP SORTED AND UNIQUE!

// data
typedef boost::shared_ptr<class Tool> ToolPtr;
typedef boost::shared_ptr<class ManualTool> ManualToolPtr;
typedef boost::shared_ptr<class DummyTool> DummyToolPtr;
typedef boost::shared_ptr<class Image> ImagePtr;
typedef boost::shared_ptr<class NavigatedVideoImage> NavigatedVideoImagePtr;
typedef boost::shared_ptr<class Data> DataPtr;
typedef boost::shared_ptr<class Mesh> MeshPtr;
typedef boost::shared_ptr<class ImageTF3D> ImageTF3DPtr;
typedef boost::shared_ptr<class ImageLUT2D> ImageLUT2DPtr;
typedef boost::shared_ptr<class ImageTFData> ImageTFDataPtr;
typedef boost::shared_ptr<class GPUImageDataBuffer> GPUImageDataBufferPtr;
typedef boost::weak_ptr<class GPUImageDataBuffer> GPUImageDataBufferWeakPtr;
typedef boost::shared_ptr<class GPUImageLutBuffer> GPUImageLutBufferPtr;
typedef boost::weak_ptr<class GPUImageLutBuffer> GPUImageLutBufferWeakPtr;
typedef boost::shared_ptr<class ProbeSector> ProbeSectorPtr;
typedef boost::shared_ptr<class FiberBundle> FiberBundlePtr;

// reps
typedef boost::shared_ptr<class Axes3D> Axes3DPtr;
typedef boost::shared_ptr<class AxesRep> AxesRepPtr;
typedef boost::shared_ptr<class CrossHair2D> CrossHair2DPtr;
typedef boost::shared_ptr<class CrossHairRep2D> CrossHairRep2DPtr;
typedef boost::shared_ptr<class DisplayTextRep> DisplayTextRepPtr;
typedef boost::shared_ptr<class FiberBundleRep> FiberBundleRepPtr;
typedef boost::shared_ptr<class GeometricRep2D> GeometricRep2DPtr;
typedef boost::shared_ptr<class GeometricRep> GeometricRepPtr;
typedef boost::shared_ptr<class GPUImageDataBuffer> GPUImageDataBufferPtr;
typedef boost::shared_ptr<class GPUImageLutBuffer> GPUImageLutBufferPtr;
typedef boost::shared_ptr<class GraphicalLine3D> GraphicalLine3DPtr;
typedef boost::shared_ptr<class GraphicalPoint3D> GraphicalPoint3DPtr;
typedef boost::shared_ptr<class GuideRep2D> GuideRep2DPtr;
typedef boost::shared_ptr<class ImageLUT2D> ImageLUT2DPtr;
typedef boost::shared_ptr<class ImageTF3D> ImageTF3DPtr;
typedef boost::shared_ptr<class LandmarkRep> LandmarkRepPtr;
typedef boost::shared_ptr<class LineSegment> LineSegmentPtr;
typedef boost::shared_ptr<class Mesh> MeshPtr;
typedef boost::shared_ptr<class OffsetPoint> OffsetPointPtr;
typedef boost::shared_ptr<class OrientationAnnotation3DRep> OrientationAnnotation3DRepPtr;
typedef boost::shared_ptr<class OrientationAnnotationRep> OrientationAnnotationRepPtr;
typedef boost::shared_ptr<class PickerRep> PickerRepPtr;
typedef boost::shared_ptr<class PointMetric> PointMetricPtr;
typedef boost::shared_ptr<class PointMetricRep> PointMetricRepPtr;
typedef boost::shared_ptr<class PointMetricRep2D> PointMetricRep2DPtr;
typedef boost::shared_ptr<class Rect3D> Rect3DPtr;
typedef boost::shared_ptr<class Rep> RepPtr;
typedef boost::shared_ptr<class SlicedImageProxy> SlicedImageProxyPtr;
typedef boost::shared_ptr<class SlicePlaneClipper> SlicePlaneClipperPtr;
typedef boost::shared_ptr<class SlicePlaneRep> SlicePlaneRepPtr;
typedef boost::shared_ptr<class SlicePlaneRep> SlicePlaneRepPtr;
typedef boost::shared_ptr<class SlicePlanes3DMarkerIn2DRep> SlicePlanes3DMarkerIn2DRepPtr;
typedef boost::shared_ptr<class SlicePlanes3DRep> SlicePlanes3DRepPtr;
typedef boost::shared_ptr<class SlicePlanesProxy> SlicePlanesProxyPtr;
typedef boost::shared_ptr<class SliceProxy> SliceProxyPtr;
typedef boost::shared_ptr<class SliceRepSW> SliceRepSWPtr;
typedef boost::shared_ptr<class SurfaceRep> SurfaceRepPtr;
typedef boost::shared_ptr<class TestVideoSource> TestVideoSourcePtr;
typedef boost::shared_ptr<class TextDisplay> TextDisplayPtr;
typedef boost::shared_ptr<class Texture3DSlicerRep> Texture3DSlicerRepPtr;
typedef boost::shared_ptr<class ToolRep2D> ToolRep2DPtr;
typedef boost::shared_ptr<class ToolRep3D> ToolRep3DPtr;
typedef boost::shared_ptr<class ToolTracer> ToolTracerPtr;
typedef boost::shared_ptr<class Tool> ToolPtr;
typedef boost::shared_ptr<class VideoFixedPlaneRep> VideoFixedPlaneRepPtr;
typedef boost::shared_ptr<class VideoSource> VideoSourcePtr;
typedef boost::shared_ptr<class View> ViewPtr;
typedef boost::shared_ptr<class VolumetricBaseRep> VolumetricBaseRepPtr;
typedef boost::shared_ptr<class VolumetricRep> VolumetricRepPtr;

// Services
typedef boost::shared_ptr<class SpaceProvider> SpaceProviderPtr;
typedef boost::shared_ptr<class DataFactory> DataFactoryPtr;
typedef boost::shared_ptr<class PatientService> PatientServicePtr;
typedef boost::shared_ptr<class ToolManager> TrackingServicePtr;
typedef boost::shared_ptr<class VideoService> VideoServicePtr;
typedef boost::shared_ptr<class ViewManager> VisualizationServicePtr;
typedef boost::shared_ptr<class StateService> StateServicePtr;
typedef boost::shared_ptr<class DataManager> DataServicePtr;

typedef boost::weak_ptr<class SpaceProvider> SpaceProviderWeakPtr;
typedef boost::weak_ptr<class DataFactory> DataFactoryWeakPtr;
typedef boost::weak_ptr<class PatientService> PatientServiceWeakPtr;
typedef boost::weak_ptr<class ToolManager> TrackingServiceWeakPtr;
typedef boost::weak_ptr<class VideoService> VideoServiceWeakPtr;
typedef boost::weak_ptr<class ViewManager> VisualizationServiceWeakPtr;
typedef boost::weak_ptr<class StateService> StateServiceWeakPtr;
typedef boost::weak_ptr<class DataManager> DataServiceWeakPtr;

// data adapters
typedef boost::shared_ptr<class StringDataAdapter> StringDataAdapterPtr;
typedef boost::shared_ptr<class DoubleDataAdapter> DoubleDataAdapterPtr;
typedef boost::shared_ptr<class BoolDataAdapter> BoolDataAdapterPtr;
typedef boost::shared_ptr<class ColorDataAdapter> ColorDataAdapterPtr;

typedef boost::shared_ptr<class StringDataAdapterXml> StringDataAdapterXmlPtr;
typedef boost::shared_ptr<class DoubleDataAdapterXml> DoubleDataAdapterXmlPtr;
typedef boost::shared_ptr<class BoolDataAdapterXml> BoolDataAdapterXmlPtr;
typedef boost::shared_ptr<class ColorDataAdapterXml> ColorDataAdapterXmlPtr;
typedef boost::shared_ptr<class DoublePairDataAdapterXml> DoublePairDataAdapterXmlPtr;

// other stuff
typedef boost::shared_ptr<class Presets> PresetsPtr;
typedef boost::shared_ptr<class ProcessedUSInputData> ProcessedUSInputDataPtr;
typedef boost::shared_ptr<class USFrameData> USFrameDataPtr;
typedef boost::shared_ptr<class ViewportListener> ViewportListenerPtr;

typedef boost::shared_ptr<class ViewGroupData> ViewGroupDataPtr;
typedef boost::shared_ptr<class ViewGroup> ViewGroupPtr;
typedef boost::shared_ptr<class ViewGroup2D> ViewGroup2DPtr;
typedef boost::shared_ptr<class ViewGroup3D> ViewGroup3DPtr;
typedef boost::shared_ptr<class ViewWrapper> ViewWrapperPtr;

typedef boost::shared_ptr<class PatientData> PatientDataPtr;
typedef boost::shared_ptr<class VideoConnectionManager> VideoConnectionManagerPtr;
typedef boost::shared_ptr<class UsReconstructionFileReader> UsReconstructionFileReaderPtr;
typedef boost::shared_ptr<class ImageLandmarksSource> ImageLandmarksSourcePtr;

typedef boost::shared_ptr<class Filter> FilterPtr;


} // namespace cx

#endif /*CX_FORWARDDECLARARATIONS_H_*/
