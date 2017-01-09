//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGpuProgram.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	GPU program compiled from GLSL and usable by OpenGL. */
    class BS_RSGL_EXPORT GLSLGpuProgram : public GpuProgramCore
    {
	public:
		~GLSLGpuProgram();

		/** @copydoc GpuProgramCore::isSupported */
		bool isSupported() const override;

		/**	Gets internal OpenGL handle to the program. */
		GLuint getGLHandle() const { return mGLHandle; }

		/** Gets an unique index for this GPU program. Each created GPU program is assigned a unique index on creation. */
		UINT32 getProgramID() const { return mProgramID; }

	private:
		friend class GLSLProgramFactory;

		GLSLGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuProgramCore::initialize */
		void initialize() override;

	private:
		UINT32 mProgramID;
		GLuint mGLHandle;

		static UINT32 mVertexShaderCount;
		static UINT32 mFragmentShaderCount;
		static UINT32 mGeometryShaderCount;
		static UINT32 mHullShaderCount;
		static UINT32 mDomainShaderCount;
		static UINT32 mComputeShaderCount;
    };

	/** @} */
}}
