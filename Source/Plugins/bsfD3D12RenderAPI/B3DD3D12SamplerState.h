//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DSamplerState.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
		 *  @{
		 */

		/** DirectX 12 implementation of a sampler state. */
		class D3D12SamplerState : public SamplerState
		{
		public:
			D3D12SamplerState(const SamplerStateCreateInformation& createInformation, GpuDevice& device);
			~D3D12SamplerState() override;

			/** @copydoc SamplerState::Initialize */
			void Initialize() override;

			/** Returns the D3D12 sampler descriptor. */
			const D3D12_SAMPLER_DESC& GetD3D12SamplerDesc() const { return mSamplerDesc; }

			/** Returns the CPU descriptor handle for this sampler. */
			D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return mDescriptorHandle; }

		private:
			D3D12GpuDevice& mDevice;
			D3D12_SAMPLER_DESC mSamplerDesc{};
			D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorHandle{};
		};

		/** @} */
	} // namespace render
} // namespace b3d
