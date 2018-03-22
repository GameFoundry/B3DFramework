//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Managers/BsGpuProgramManager.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Factory class that deals with creating GLSL GPU programs. */
	class GLSLProgramFactory : public GpuProgramFactory
	{
	public:
		/** @copydoc GpuProgramFactory::create(const GPU_PROGRAM_DESC&, GpuDeviceFlags) */
		SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::create(GpuProgramType, GpuDeviceFlags) */
		SPtr<GpuProgram> create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::compile(const GPU_PROGRAM_DESC&, GpuDeviceFlags) */
		SPtr<GpuProgramBytecode> compileBytecode(const GPU_PROGRAM_DESC& desc) override;
	protected:
		static const String LANGUAGE_NAME;
	};

	/** @} */
}}