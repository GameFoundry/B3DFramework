//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "BsGLBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a GPU parameter buffer (Uniform buffer). */
	class GLGpuParamBlockBuffer : public GpuParamBlockBuffer
	{
	public:
		GLGpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);
		~GLGpuParamBlockBuffer();

		/**	Returns internal OpenGL uniform buffer handle. */
		GLuint getGLBufferId() const { return static_cast<GLBuffer*>(mBuffer)->getGLBufferId(); }
	protected:
		/** @copydoc GpuParamBlockBuffer::initialize */
		void initialize() override ;
	};

	/** @} */
}}
