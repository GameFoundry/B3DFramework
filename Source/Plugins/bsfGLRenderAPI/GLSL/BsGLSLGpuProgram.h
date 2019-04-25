//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsGpuProgram.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	GPU program compiled from GLSL and usable by OpenGL. */
	class GLSLGpuProgram : public GpuProgram
	{
	public:
		~GLSLGpuProgram();

		/** @copydoc GpuProgram::isSupported */
		bool isSupported() const override;

		/**	Gets internal OpenGL handle to the program. */
		GLuint getGLHandle() const { return mGLHandle; }

		/** Gets an unique index for this GPU program. Each created GPU program is assigned a unique index on creation. */
		UINT32 getProgramID() const { return mProgramID; }

	private:
		friend class GLSLProgramFactory;

		GLSLGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuProgram::initialize */
		void initialize() override;

	private:
		UINT32 mProgramID = 0;
		GLuint mGLHandle = 0;

		static UINT32 sVertexShaderCount;
		static UINT32 sFragmentShaderCount;
		static UINT32 sGeometryShaderCount;
		static UINT32 sHullShaderCount;
		static UINT32 sDomainShaderCount;
		static UINT32 sComputeShaderCount;
	};

	/** Identifier of the compiler used for compiling OpenGL GPU programs. */
	static constexpr const char* OPENGL_COMPILER_ID = "OpenGL";

	/** @} */
}}
