//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLContext.h"
#include "BsGLPixelBuffer.h"
#include "BsPixelData.h"

namespace bs 
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Describes OpenGL frame buffer surface. */
    struct BS_RSGL_EXPORT GLSurfaceDesc
    {
    public:
		GLSurfaceDesc() 
			:zoffset(0), numSamples(0), allLayers(false)
		{ }

		SPtr<GLPixelBuffer> buffer;
		UINT32 zoffset;
		UINT32 numSamples;
		bool allLayers;
    };

	/**
	 * Manages an OpenGL frame-buffer object. Frame buffer object is used as a rendering destination in the render system
	 * pipeline, and it may consist out of one or multiple color surfaces and an optional depth/stencil surface.
	 */
    class BS_RSGL_EXPORT GLFrameBufferObject
    {
    public:
        GLFrameBufferObject();
        ~GLFrameBufferObject();

		/**
		 * Binds a color surface to the specific attachment point.
		 *
		 * @param[in]	attachment	Attachment point index in the range [0, BS_MAX_MULTIPLE_RENDER_TARGETS).
		 * @param[in]	target		Description of the color surface to attach.
		 *
		 * @note	
		 * Multisample counts of all surfaces must match.
		 * 0th attachment must be bound in order for the object to be usable, rest are optional.
		 */
        void bindSurface(UINT32 attachment, const GLSurfaceDesc& target);

		/**
		 * Unbinds the attachment at the specified attachment index.
		 *
		 * @param[in]	attachment	Attachment point index in the range [0, BS_MAX_MULTIPLE_RENDER_TARGETS).
		 */
        void unbindSurface(UINT32 attachment);

		/**
		 * Binds a depth/stencil buffer.
		 *
		 * @note	
		 * Multisample counts of depth/stencil and color surfaces must match.
		 * Binding a depth/stencil buffer is optional.
		 */
		void bindDepthStencil(SPtr<GLPixelBuffer> depthStencilBuffer);

		/**	Unbinds a depth stencil buffer. */
		void unbindDepthStencil();
        
		/** Binds the frame buffer object to the OpenGL pipeline, making it used for any further rendering operations. */
        void bind();

		/** Checks is the color buffer at the specified index bound. */
		bool hasColorBuffer(UINT32 idx) const { return mColor[idx].buffer != nullptr; }

		/**	Returns internal OpenGL frame buffer id. */
		GLuint getGLFBOID() const { return mFB; }

    private:
		/**	Rebuilds internal frame buffer object. Should be called whenever surfaces change. */
        void rebuild();

	private:
        GLuint mFB;

        GLSurfaceDesc mColor[BS_MAX_MULTIPLE_RENDER_TARGETS];
		SPtr<GLPixelBuffer> mDepthStencilBuffer;
    };

	/** @} */
}