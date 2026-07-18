//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DVertexDescription.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		class D3D12VertexInput;
		class D3D12GpuPipelineParameterLayout;

		/**
		 * Key identifying a single D3D12 pipeline state variant. D3D12 pipeline state objects bake in the render
		 * target formats, primitive topology type and vertex input layout, which the engine-level pipeline state
		 * does not know upfront, so concrete pipelines are created lazily per encountered combination at draw time.
		 */
		struct D3D12PipelineVariantKey
		{
			DXGI_FORMAT RenderTargetFormats[8] = {};
			DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			u32 RenderTargetCount = 0;
			u32 SampleCount = 1;
			D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			u32 VertexInputId = 0;

			bool operator==(const D3D12PipelineVariantKey& other) const;

			/** Hash function usable with unordered containers. */
			struct Hash
			{
				size_t operator()(const D3D12PipelineVariantKey& key) const;
			};
		};

		/** DirectX 12 implementation of a graphics pipeline state. */
		class D3D12GpuGraphicsPipelineState : public GpuGraphicsPipelineState
		{
		public:
			D3D12GpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuDevice& device);
			~D3D12GpuGraphicsPipelineState() override;

			/** @copydoc GpuGraphicsPipelineState::Initialize */
			void Initialize() override;

			/**
			 * Returns (creating on first use) the pipeline state object variant matching the provided render target
			 * formats, topology and vertex input. The key's VertexInputId must identify the provided vertex input.
			 * Returns null if creation failed.
			 */
			ID3D12PipelineState* FindOrCreatePipeline(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput);

			/** Returns the root signature used by this pipeline. */
			ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }

			/** Returns the D3D12 parameter layout backing the root signature. */
			D3D12GpuPipelineParameterLayout* GetD3D12ParameterLayout() const;

			/** Returns a description of the vertex element inputs expected by the pipeline's vertex shader. */
			const TShared<VertexDescription>& GetInputDeclaration() const { return mVertexDescription; }

		private:
			/** Creates the D3D12 pipeline state object variant for the given key and vertex input. */
			ComPtr<ID3D12PipelineState> CreatePipelineState(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput);

			UnorderedMap<D3D12PipelineVariantKey, ComPtr<ID3D12PipelineState>, D3D12PipelineVariantKey::Hash> mPipelines;
			ComPtr<ID3D12RootSignature> mRootSignature;
			TShared<VertexDescription> mVertexDescription;
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

			/** Returns the D3D12 parameter layout backing the root signature. */
			D3D12GpuPipelineParameterLayout* GetD3D12ParameterLayout() const;

		private:
			/** Creates the D3D12 pipeline state object. */
			void CreatePipelineState();

			ComPtr<ID3D12PipelineState> mPipelineState;
			ComPtr<ID3D12RootSignature> mRootSignature;
		};

		/** @} */
	} // namespace render
} // namespace b3d
