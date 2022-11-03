//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLContext.h"
#include "BsGLPixelBuffer.h"
#include "Image/BsPixelData.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Describes OpenGL frame buffer surface. */
		struct GLSurfaceDesc
		{
		public:
			GLSurfaceDesc() = default;

			SPtr<GLPixelBuffer> Buffer;
			u32 Zoffset = 0;
			u32 NumSamples = 0;
			bool AllLayers = false;
		};

		/**
		 * Manages an OpenGL frame-buffer object. Frame buffer object is used as a rendering destination in the render system
		 * pipeline, and it may consist out of one or multiple color surfaces and an optional depth/stencil surface.
		 */
		class GLFrameBufferObject
		{
		public:
			GLFrameBufferObject();
			~GLFrameBufferObject();

			/**
			 * Binds a color surface to the specific attachment point. Call rebuild() to apply changes.
			 *
			 * @param[in]	attachment	Attachment point index in the range [0, B3D_MAXIMUM_RENDER_TARGET_COUNT).
			 * @param[in]	target		Description of the color surface to attach.
			 *
			 * @note
			 * Multisample counts of all surfaces must match.
			 * 0th attachment must be bound in order for the object to be usable, rest are optional.
			 */
			void BindSurface(u32 attachment, const GLSurfaceDesc& target);

			/**
			 * Unbinds the attachment at the specified attachment index. Call rebuild() to apply changes.
			 *
			 * @param[in]	attachment	Attachment point index in the range [0, B3D_MAXIMUM_RENDER_TARGET_COUNT).
			 */
			void UnbindSurface(u32 attachment);

			/**
			 * Binds a depth/stencil buffer. Call rebuild() to apply changes.
			 *
			 * @note
			 * Multisample counts of depth/stencil and color surfaces must match.
			 * Binding a depth/stencil buffer is optional.
			 */
			void BindDepthStencil(SPtr<GLPixelBuffer> depthStencilBuffer, bool allLayers);

			/**	Unbinds a depth stencil buffer. Call rebuild() to apply changes. */
			void UnbindDepthStencil();

			/**	Rebuilds internal frame buffer object. Should be called whenever surfaces changes or is bound/unbound. */
			void Rebuild();

			/** Binds the frame buffer object to the OpenGL pipeline, making it used for any further rendering operations. */
			void Bind();

			/** Checks is the color buffer at the specified index bound. */
			bool HasColorBuffer(u32 idx) const { return mColor[idx].Buffer != nullptr; }

			/**	Returns internal OpenGL frame buffer id. */
			GLuint GetGlfboid() const { return mFB; }

		private:
			GLuint mFB;

			GLSurfaceDesc mColor[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			SPtr<GLPixelBuffer> mDepthStencilBuffer;
			bool mDepthStencilAllLayers = false;
		};

		/** @} */
	} // namespace ct
} // namespace bs
