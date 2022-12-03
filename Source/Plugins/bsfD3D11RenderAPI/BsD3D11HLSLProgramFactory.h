//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Managers/BsGpuProgramManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Handles creation of DirectX 11 HLSL GPU programs. */
		class D3D11HLSLProgramFactory : public GpuProgramFactory
		{
		public:
			D3D11HLSLProgramFactory() = default;
			~D3D11HLSLProgramFactory() = default;

			SPtr<GpuProgram> Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuProgram> Create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& desc) override;

		protected:
			static const String LANGUAGE_NAME;
		};

		/** @} */
	} // namespace ct
} // namespace bs
