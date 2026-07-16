//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "Utility/B3DUtil.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;
		class MetalVertexInput;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Key used to cache per-format variants of a Metal graphics pipeline state.
		 *
		 * Metal fuses attachment pixel formats and the vertex descriptor into the pipeline object, so a
		 * single engine-level pipeline state may expand into several @c MTLRenderPipelineState objects
		 * depending on the render target, draw topology and vertex-buffer layout it is bound against.
		 */
		struct MetalPipelineVariantKey
		{
			/**
			 * A'4: one @c u16 slot per color attachment (up to @c B3D_MAXIMUM_RENDER_TARGET_COUNT).
			 * Previously packed as 8 bits per slot inside a @c u64, which truncated
			 * @c MTLPixelFormat values > 255 (e.g. @c BGR10_XR=554, @c Depth32Float_Stencil8=260,
			 * several ASTC HDR variants) and caused distinct formats to alias into the same cache
			 * entry. @c MTLPixelFormat values fit inside @c u16 as of the public headers, so @c u16
			 * is sufficient without growing the key further.
			 */
			u16 ColorFormats[B3D_MAXIMUM_RENDER_TARGET_COUNT] = {};
			u32 DepthFormat = 0; /**< MTLPixelFormat of the depth attachment, or 0 if none. */
			u32 StencilFormat = 0; /**< MTLPixelFormat of the stencil attachment, or 0 if none. */
			u16 SampleCount = 1;
			u16 TopologyClass = 0; /**< MTLPrimitiveTopologyClass value. */

			/**
			 * Identifier of the MetalVertexInput (vertex-buffer layout resolved against the vertex
			 * shader inputs, see MetalVertexInputManager) this variant is compiled with. Metal bakes
			 * the vertex descriptor into the pipeline object, so the same engine pipeline bound
			 * against differently-laid-out vertex buffers expands into distinct
			 * @c MTLRenderPipelineState objects. Zero when the pipeline consumes no vertex input.
			 */
			u32 VertexInputId = 0;

			bool operator==(const MetalPipelineVariantKey& rhs) const
			{
				for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
				{
					if (ColorFormats[attachmentIndex] != rhs.ColorFormats[attachmentIndex])
						return false;
				}
				return DepthFormat == rhs.DepthFormat
					&& StencilFormat == rhs.StencilFormat
					&& SampleCount == rhs.SampleCount
					&& TopologyClass == rhs.TopologyClass
					&& VertexInputId == rhs.VertexInputId;
			}
		};

		struct MetalPipelineVariantKeyHash
		{
			size_t operator()(const MetalPipelineVariantKey& key) const
			{
				size_t h = 0;
				for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
					B3DCombineHash(h, key.ColorFormats[attachmentIndex]);
				B3DCombineHash(h, key.DepthFormat);
				B3DCombineHash(h, key.StencilFormat);
				B3DCombineHash(h, key.SampleCount);
				B3DCombineHash(h, key.TopologyClass);
				B3DCombineHash(h, key.VertexInputId);
				return h;
			}
		};

		/**
		 * Metal implementation of a graphics pipeline state.
		 *
		 * Initialize() builds the render-pass-independent state (depth-stencil state, cached blend and
		 * rasterizer state) and publishes the vertex program's input declaration. The actual
		 * @c MTLRenderPipelineState is created lazily at bind time via GetOrCreateMetalPipeline()
		 * because Metal fuses attachment formats and the vertex descriptor into the pipeline object,
		 * and the engine does not know either until a render pass is entered and vertex buffers are
		 * bound. The vertex descriptor is supplied per-variant as a MetalVertexInput resolved by
		 * MetalVertexInputManager from the bound vertex-buffer VertexDescription and this pipeline's
		 * GetInputDeclaration(), mirroring how VulkanGpuCommandBuffer::BindGraphicsPipeline feeds
		 * VulkanVertexInput into VulkanGpuGraphicsPipelineState::FindOrCreateVulkanResource.
		 */
		class MetalGpuGraphicsPipelineState : public GpuGraphicsPipelineState
		{
		public:
			MetalGpuGraphicsPipelineState(MetalGpuDevice& gpuDevice, const GpuGraphicsPipelineStateCreateInformation& createInformation);
			~MetalGpuGraphicsPipelineState() override;

			void Initialize() override;

			/** Returns the vertex input declaration from the vertex GPU program bound on the pipeline. */
			const TShared<VertexDescription>& GetInputDeclaration() const { return mVertexDescription; }

#ifdef __OBJC__
			/** Returns the depth-stencil state object; remains valid for the pipeline's lifetime. */
			id<MTLDepthStencilState> GetMetalDepthStencilState() const;

			/**
			 * Returns a cached (or freshly created) render pipeline state for the given attachment
			 * format / topology / vertex-input combination. May return nil if the pipeline compile
			 * failed, or if the Metal device is unavailable at the time of the call — in the latter
			 * case the cache is left untouched so a subsequent call retries the compile once the
			 * device comes back.
			 *
			 * @p vertexInput must be the MetalVertexInput whose GetId() was written into
			 * @p key.VertexInputId (null when the pipeline consumes no vertex input, with
			 * @p key.VertexInputId == 0). Its vertex descriptor is copied into the pipeline
			 * descriptor, so the object only needs to stay alive for the duration of this call.
			 *
			 * Blocks until the compile completes: if the pipeline was previously prewarmed via
			 * @c Prewarm the call returns as soon as the already-in-flight completion handler
			 * publishes the result, otherwise this kicks off the compile and waits on the same
			 * completion handler.
			 */
			id<MTLRenderPipelineState> GetOrCreateMetalPipeline(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput);
#endif

			/**
			 * Kicks off an async compile of the pipeline variant identified by @p key without blocking.
			 * If the variant is already compiled, cached as in-flight, or previously failed, this is a
			 * no-op. A subsequent @c GetOrCreateMetalPipeline call for the same key will pick up the
			 * already-in-flight compile instead of re-issuing it.
			 *
			 * @p vertexInput follows the same contract as in @c GetOrCreateMetalPipeline.
			 *
			 * Prewarming is the intended happy-path usage of the async PSO-compile pipeline: drive every
			 * pipeline through a warmup loop at level-load time so draws never hit a cold compile at
			 * bind. Safe to call from any thread.
			 */
			void Prewarm(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput);

			/** Returns the Metal cull mode computed from the engine rasterizer state. */
			u32 GetCullMode() const { return mCullMode; }

			/** Returns the Metal front-face winding order. */
			u32 GetWinding() const { return mWinding; }

			/** Returns the Metal triangle fill mode. */
			u32 GetFillMode() const { return mFillMode; }

			/** Returns constant depth bias applied on the encoder. */
			float GetDepthBias() const { return mDepthBias; }

			/** Returns slope-scaled depth bias applied on the encoder. */
			float GetSlopeScaledDepthBias() const { return mSlopeScaledDepthBias; }

			/** Returns depth bias clamp applied on the encoder. */
			float GetDepthBiasClamp() const { return mDepthBiasClamp; }

			/** Returns whether scissor testing is enabled in the pipeline. */
			bool IsScissorEnabled() const { return mScissorEnabled; }

			/**
			 * Returns the base Metal buffer-slot index at which vertex-stream buffers are expected.
			 *
			 * Argument buffers for parameter sets occupy the low buffer slots @c [0, setCount); vertex
			 * streams start at the fixed @c kMetalVertexBufferSlotBase so the two binding tables never
			 * collide and cached MTLVertexDescriptors stay valid across pipelines. Both the per-variant
			 * vertex descriptor and the command buffer's @c setVertexBuffer calls offset stream indices
			 * by this value.
			 */
			u32 GetVertexBufferBaseIndex() const { return mVertexBufferBaseIndex; }

		private:
			struct Impl;

#ifdef __OBJC__
			/**
			 * Inserts a pending cache entry for @p key (if one doesn't already exist) and fires the async
			 * @c newRenderPipelineStateWithDescriptor:completionHandler: call, using @p vertexInput's
			 * descriptor as the variant's vertex input. Returns true if a new compile was actually
			 * dispatched; false if the key was already in the cache (ready or pending). Called by both
			 * @c Prewarm (no wait) and @c GetOrCreateMetalPipeline (wait after dispatch).
			 */
			bool StartCompile(const MetalPipelineVariantKey& key, const TShared<MetalVertexInput>& vertexInput);
#endif

			MetalGpuDevice& mGpuDevice;
			TUnique<Impl> mImpl;

			// Shader-side vertex input declaration, published in Initialize(). Resolved against the
			// bound vertex-buffer VertexDescription by the command buffer at bind time via
			// MetalVertexInputManager (mirrors VulkanGpuGraphicsPipelineState::mVertexDescription).
			TShared<VertexDescription> mVertexDescription;

			// Cached rasterizer state applied on the render encoder at bind time.
			u32 mCullMode = 0;
			u32 mWinding = 0;
			u32 mFillMode = 0;
			float mDepthBias = 0.0f;
			float mSlopeScaledDepthBias = 0.0f;
			float mDepthBiasClamp = 0.0f;
			bool mScissorEnabled = false;
			u32 mVertexBufferBaseIndex = 0;
		};

		/**
		 * Metal implementation of a compute pipeline state.
		 *
		 * Compute pipelines are not render-pass-dependent, so the @c MTLComputePipelineState is built
		 * eagerly in Initialize().
		 */
		class MetalGpuComputePipelineState : public GpuComputePipelineState
		{
		public:
			MetalGpuComputePipelineState(MetalGpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation);
			~MetalGpuComputePipelineState() override;

			void Initialize() override;

			/** Returns the compute workgroup size as reported by the bound GPU program. */
			const u32* GetWorkgroupSize() const { return mWorkgroupSize; }

			/**
			 * Kicks off the async compute-pipeline compile if @c Initialize has not already fired it.
			 * A no-op if the compile is in flight or already complete. Useful for warming compute PSOs
			 * from the resource loader so the first dispatch does not block on the MSL compile.
			 */
			void Prewarm();

#ifdef __OBJC__
			/**
			 * Returns the underlying compute pipeline state; may be nil if compilation failed.
			 * Blocks on the async compile if it has not landed yet.
			 */
			id<MTLComputePipelineState> GetMetalPipeline() const;
#endif

		private:
			struct Impl;

			MetalGpuDevice& mGpuDevice;
			TUnique<Impl> mImpl;
			u32 mWorkgroupSize[3] = { 1, 1, 1 };
		};

		/** @} */
	} // namespace render
} // namespace b3d
