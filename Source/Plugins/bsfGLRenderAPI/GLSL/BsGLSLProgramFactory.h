//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Managers/BsGpuProgramManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Factory class that deals with creating GLSL GPU programs. */
		class GLSLProgramFactory : public GpuProgramFactory
		{
		public:
			/** @copydoc GpuProgramFactory::Create(const GPU_PROGRAM_DESC&, GpuDeviceFlags) */
			SPtr<GpuProgram> Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GpuProgramFactory::Create(GpuProgramType, GpuDeviceFlags) */
			SPtr<GpuProgram> Create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GpuProgramFactory::compileBytecode(const GPU_PROGRAM_DESC&) */
			SPtr<GpuProgramBytecode> CompileBytecode(const GPU_PROGRAM_DESC& desc);

		protected:
			static const String LANGUAGE_NAME;
		};

		/** @} */
	} // namespace ct
} // namespace bs
