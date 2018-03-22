//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Managers/BsGpuProgramManager.h"

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

		/** @copydoc GpuProgramFactory::create(const GPU_PROGRAM_DESC&, GpuDeviceFlags) */
		SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::create(GpuProgramType, GpuDeviceFlags) */
		SPtr<GpuProgram> create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::compileBytecode(const GPU_PROGRAM_DESC&) */
		SPtr<GpuProgramBytecode> compileBytecode(const GPU_PROGRAM_DESC& desc) override;
	protected:
		static const String LANGUAGE_NAME;
	};

	/** @} */
}}