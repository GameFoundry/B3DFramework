//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
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
			DXGI_FORMAT RenderTargetFormats[B3D_MAXIMUM_RENDER_TARGET_COUNT] = {};
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

			void Initialize() override;

			/**
			 * Returns (creating on first use) the pipeline state object variant matching the provided render target
			 * formats, topology and vertex input. The key's VertexInputId must identify the provided vertex input.
			 * Returns null if creation failed.
			 */
			D3D12Pipeline* FindOrCreatePipeline(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput);

			/** Returns the tracked root signature used by this pipeline. */
			D3D12RootSignature* GetRootSignature() const;

			/** Returns the D3D12 parameter layout backing the root signature. */
			D3D12GpuPipelineParameterLayout* GetD3D12ParameterLayout() const;

			/** Returns a description of the vertex element inputs expected by the pipeline's vertex shader. */
			const TShared<VertexDescription>& GetInputDeclaration() const { return mVertexDescription; }

		private:
			/** Creates the D3D12 pipeline state object variant for the given key and vertex input. */
			D3D12Pipeline* CreatePipelineState(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput);

			UnorderedMap<D3D12PipelineVariantKey, D3D12Pipeline*, D3D12PipelineVariantKey::Hash> mPipelines;
			TShared<VertexDescription> mVertexDescription;
		};

		/** DirectX 12 implementation of a compute pipeline state. */
		class D3D12GpuComputePipelineState : public GpuComputePipelineState
		{
		public:
			D3D12GpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuDevice& device);
			~D3D12GpuComputePipelineState() override;

			void Initialize() override;

			/** Returns the tracked D3D12 pipeline state object. */
			D3D12Pipeline* GetD3D12PipelineState() const { return mPipelineState; }

			/** Returns the tracked root signature used by this pipeline. */
			D3D12RootSignature* GetRootSignature() const;

			/** Returns the D3D12 parameter layout backing the root signature. */
			D3D12GpuPipelineParameterLayout* GetD3D12ParameterLayout() const;

		private:
			/** Creates the D3D12 pipeline state object. */
			void CreatePipelineState();

			D3D12Pipeline* mPipelineState = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
