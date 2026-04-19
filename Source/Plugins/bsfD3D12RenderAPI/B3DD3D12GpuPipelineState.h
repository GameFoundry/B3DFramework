//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DGpuPipelineState.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
		 *  @{
		 */

		/** DirectX 12 implementation of a graphics pipeline state. */
		class D3D12GpuGraphicsPipelineState : public GpuGraphicsPipelineState
		{
		public:
			D3D12GpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuDevice& device);
			~D3D12GpuGraphicsPipelineState() override;

			/** @copydoc GpuGraphicsPipelineState::Initialize */
			void Initialize() override;

			/** Returns the D3D12 pipeline state object. */
			ID3D12PipelineState* GetD3D12PipelineState() const { return mPipelineState.Get(); }

			/** Returns the root signature used by this pipeline. */
			ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

			/** Returns the primitive topology type for this pipeline. */
			D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return mPrimitiveTopology; }

		private:
			/** Creates the D3D12 pipeline state object. */
			void CreatePipelineState();

			ComPtr<ID3D12PipelineState> mPipelineState;
			ComPtr<ID3D12RootSignature> mRootSignature;
			D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		};

		/** DirectX 12 implementation of a compute pipeline state. */
		class D3D12GpuComputePipelineState : public GpuComputePipelineState
		{
		public:
			D3D12GpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuDevice& device);
			~D3D12GpuComputePipelineState() override;

			/** @copydoc GpuComputePipelineState::Initialize */
			void Initialize() override;

			/** Returns the D3D12 pipeline state object. */
			ID3D12PipelineState* GetD3D12PipelineState() const { return mPipelineState.Get(); }

			/** Returns the root signature used by this pipeline. */
			ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

		private:
			/** Creates the D3D12 pipeline state object. */
			void CreatePipelineState();

			ComPtr<ID3D12PipelineState> mPipelineState;
			ComPtr<ID3D12RootSignature> mRootSignature;
		};

		/** @} */
	} // namespace render
} // namespace b3d
