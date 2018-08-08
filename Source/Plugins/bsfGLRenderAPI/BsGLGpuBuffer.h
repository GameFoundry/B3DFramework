//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "BsGLHardwareBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a generic GPU buffer. */
	class GLGpuBuffer : public GpuBuffer
	{
	public:
		~GLGpuBuffer();

		/**	
		 * Returns internal OpenGL buffer ID. If binding the buffer to the pipeline, bind the texture using
		 * getGLTextureId() instead. 
		 */
		GLuint getGLBufferId() const { return static_cast<GLHardwareBuffer*>(mBuffer)->getGLBufferId(); }

		/**	Returns internal OpenGL texture ID. */
		GLuint getGLTextureId() const { return mTextureID; }

		/** Returns the internal OpenGL format used by the elements of the buffer. */
		GLuint getGLFormat() const { return mFormat; }

	protected:
		friend class GLHardwareBufferManager;

		GLGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
		GLGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer);

		/** @copydoc GpuBuffer::initialize */
		void initialize() override;

		GLuint mTextureID = 0;
		GLenum mFormat = 0;
	};

	/** @} */
}}