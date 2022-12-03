//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsGpuProgram.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	GPU program compiled from GLSL and usable by OpenGL. */
		class GLSLGpuProgram : public GpuProgram
		{
		public:
			~GLSLGpuProgram();

			bool IsSupported() const override;

			/**	Gets internal OpenGL handle to the program. */
			GLuint GetGlHandle() const { return mGLHandle; }

			/** Gets an unique index for this GPU program. Each created GPU program is assigned a unique index on creation. */
			u32 GetProgramId() const { return mProgramID; }

		private:
			friend class GLSLProgramFactory;

			GLSLGpuProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void Initialize() override;

		private:
			u32 mProgramID = 0;
			GLuint mGLHandle = 0;

			static u32 sVertexShaderCount;
			static u32 sFragmentShaderCount;
			static u32 sGeometryShaderCount;
			static u32 sHullShaderCount;
			static u32 sDomainShaderCount;
			static u32 sComputeShaderCount;
		};

		/** Identifier of the compiler used for compiling OpenGL GPU programs. */
		static constexpr const char* kOpenglCompilerId = "OpenGL";

		/** @} */
	} // namespace ct
} // namespace bs
