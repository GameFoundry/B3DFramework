//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGpuProgramManager.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Factory class that deals with creating GLSL GPU programs. */
    class BS_RSGL_EXPORT GLSLProgramFactory : public GpuProgramFactory
    {
    public:
		/** @copydoc GpuProgramFactory::getLanguage */
		const String& getLanguage() const override;

		/** @copydoc GpuProgramFactory::create(const GPU_PROGRAM_DESC&, GpuDeviceFlags) */
		SPtr<GpuProgramCore> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::create(GpuProgramType, GpuDeviceFlags) */
		SPtr<GpuProgramCore> create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

	protected:
		static const String LANGUAGE_NAME;
    };

	/** @} */
}