//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "BsGLHardwareBuffer.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	OpenGL implementation of a GPU parameter buffer (Uniform buffer). */
		class GLGpuParamBlockBuffer : public GpuParamBlockBuffer
		{
		public:
			GLGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);
			~GLGpuParamBlockBuffer();

			/**	Returns internal OpenGL uniform buffer handle. */
			GLuint GetGlBufferId() const { return static_cast<GLHardwareBuffer*>(mBuffer)->GetGlBufferId(); }

		protected:
			/** @copydoc GpuParamBlockBuffer::initialize */
			void Initialize();
		};

		/** @} */
	} // namespace ct
} // namespace bs
