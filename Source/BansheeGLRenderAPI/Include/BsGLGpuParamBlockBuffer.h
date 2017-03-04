//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGpuParamBlockBuffer.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a GPU parameter buffer (Uniform buffer). */
	class GLGpuParamBlockBuffer : public GpuParamBlockBuffer
	{
	public:
		GLGpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage, GpuDeviceFlags deviceMask);
		~GLGpuParamBlockBuffer();

		/** @copydoc GpuParamBlockBuffer::writeToGPU */
		void writeToGPU(const UINT8* data, UINT32 queueIdx = 0) override;

		/**	Returns internal OpenGL uniform buffer handle. */
		GLuint getGLHandle() const { return mGLHandle; }
	protected:
		/** @copydoc GpuParamBlockBuffer::initialize */
		void initialize() override ;

	private:
		GLuint mGLHandle;
	};

	/** @} */
}}