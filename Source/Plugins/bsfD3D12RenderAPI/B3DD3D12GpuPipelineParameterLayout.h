//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
		 *  @{
		 */

		/** DirectX 12 implementation of GPU pipeline parameter layout. */
		class D3D12GpuPipelineParameterLayout : public GpuPipelineParameterLayout
		{
		public:
			D3D12GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation, D3D12GpuDevice& device);
			~D3D12GpuPipelineParameterLayout() override;

			/** Returns the D3D12 root signature. */
			ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

		private:
			/** Creates the D3D12 root signature. */
			void CreateRootSignature();

			D3D12GpuDevice& mDevice;
			ComPtr<ID3D12RootSignature> mRootSignature;
		};

		/** @} */
	} // namespace render
} // namespace b3d
