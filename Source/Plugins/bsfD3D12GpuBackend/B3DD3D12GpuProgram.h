//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuProgram.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** DirectX 12 implementation of a GPU program. */
		class D3D12GpuProgram : public GpuProgram
		{
		public:
			D3D12GpuProgram(const GpuProgramCreateInformation& createInformation, GpuDevice& device);
			~D3D12GpuProgram() override;

			/** @copydoc GpuProgram::Initialize */
			void Initialize() override;

			/** Returns the compiled shader bytecode descriptor. */
			const D3D12_SHADER_BYTECODE& GetShaderBytecode() const { return mShaderBytecode; }

			/** Returns the compiled shader blob. */
			ID3DBlob* GetShaderBlob() const { return mShaderBlob.Get(); }

		private:
			GpuDevice& mGpuDevice;
			ComPtr<ID3DBlob> mShaderBlob;
			D3D12_SHADER_BYTECODE mShaderBytecode{};
		};

		/** @} */
	} // namespace render
} // namespace b3d
