/*
 * vmTextureSlicePainter.h
 *
 *  Created on: Oct 13, 2009
 *      Author: petterw
 */

#ifndef SSCTEXTURESLICEPAINTER_H_
#define SSCTEXTURESLICEPAINTER_H_

#include <boost/shared_ptr.hpp>
#include <vtkOpenGLRepresentationPainter.h>
#include <QString>

#include "vtkForwardDeclarations.h"
#include "sscForwardDeclarations.h"

//---------------------------------------------------------
namespace ssc
{

#ifndef WIN32

/**
 * \class TextureSlicePainter
 *
 * \brief Helper class for GPU rendering of slices.
 * Used by Texture3DSlicerRep.
 */
class TextureSlicePainter : public vtkOpenGLRepresentationPainter
{
public:
	static TextureSlicePainter* New();
	vtkTypeRevisionMacro(TextureSlicePainter, vtkOpenGLRepresentationPainter);

	virtual void ReleaseGraphicsResources(vtkWindow *);
	void PrintSelf(ostream& os, vtkIndent indent);

	void setShaderFile(QString shaderFile);
	void SetColorAttribute(int index, float window, float level, float llr, float alpha);
	void SetVolumeBuffer(int index, ssc::GPUImageDataBufferPtr buffer);
	void SetLutBuffer(int index, ssc::GPUImageLutBufferPtr buffer);
	void releaseGraphicsResources(int index);
protected:
	TextureSlicePainter();
	virtual ~TextureSlicePainter();
	virtual void PrepareForRendering(vtkRenderer*, vtkActor*);
	void RenderInternal(vtkRenderer* renderer, vtkActor* actor, unsigned long typeflags, bool forceCompileOnly);

	bool CanRender(vtkRenderer*, vtkActor*);
	bool LoadRequiredExtensions(vtkOpenGLExtensionManager* mgr);
	bool LoadRequiredExtension(vtkOpenGLExtensionManager* mgr, QString id);
	QString loadShaderFile(QString shaderFile);

	class vtkInternals;
	vtkInternals* mInternals;
	QString mSource;
	QString mShaderFile;
};

#endif // WIN32


//---------------------------------------------------------
}//end namespace
//---------------------------------------------------------
#endif /* SSCTEXTURESLICEPAINTER_H_ */
