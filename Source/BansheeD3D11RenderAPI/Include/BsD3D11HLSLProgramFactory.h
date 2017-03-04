//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuProgramManager.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Handles creation of DirectX 11 HLSL GPU programs. */
	class D3D11HLSLProgramFactory : public GpuProgramFactory
	{
	public:
		D3D11HLSLProgramFactory();
		~D3D11HLSLProgramFactory();

		/** @copydoc GpuProgramFactory::getLanguage */
		const String& getLanguage() const override;

		/** @copydoc GpuProgramFactory::create(const GPU_PROGRAM_DESC&, GpuDeviceFlags) */
		SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::create(GpuProgramType, GpuDeviceFlags) */
		SPtr<GpuProgram> create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

	protected:
		static const String LANGUAGE_NAME;
	};

	/** @} */
}}