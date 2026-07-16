//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalBarrierHelper.h"
#include "B3DMetalResourceTracker.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

// Generic barrier-helper method definitions, followed by the explicit instantiation for the Metal
// barrier helper. Included here (after the complete MetalBarrierHelper and MetalResourceTracker
// types are available) so the single instantiation lives in this translation unit. The header
// carries a matching `extern template` to suppress implicit instantiation elsewhere.
#include "GpuBackend/B3DGpuBarrierHelper.inl"

template class b3d::render::TGpuBarrierHelper<b3d::render::MetalBarrierHelper>;

namespace b3d::render
{
	MetalBarrierHelper::MetalBarrierHelper(MetalResourceTracker* resourceTracker)
		: TGpuBarrierHelper<MetalBarrierHelper>(resourceTracker)
	{ }

	void MetalBarrierHelper::RecordBufferBarrier(IGpuBufferResource* buffer, const GpuHazardBarrier& barrier)
	{
		// Scope-based accumulation: the native barrier synchronizes the whole buffer category, so the
		// specific resource and access direction only matter to the base's tracker bookkeeping.
		(void)buffer;

		mHasBufferBarriers = true;
		mCombinedSourceStages |= barrier.SourceStages;
		mCombinedDestinationStages |= barrier.DestinationStages;
	}

	void MetalBarrierHelper::RecordSubresourceBarrier(IGpuImageResource* image,
		const GpuTextureSubresourceRange& subresourceRange, const GpuHazardBarrier& barrier,
		GpuImageLayout oldLayout, GpuImageLayout newLayout)
	{
		// Metal has no image layouts — the transition itself is a no-op natively. The base records the
		// layout bookkeeping (mImageLayoutTracking) and pushes it back into the tracker after Execute.
		(void)image;
		(void)subresourceRange;
		(void)oldLayout;
		(void)newLayout;

		mHasTextureBarriers = true;

		// Barriers touching attachment stages also need the render-target scope so in-pass
		// attachment writes are visible to subsequent reads.
		const GpuStageFlags kAttachmentStages = GpuStageFlag::ColorAttachment | GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
		if(barrier.SourceStages.IsSetAny(kAttachmentStages) || barrier.DestinationStages.IsSetAny(kAttachmentStages))
			mHasRenderTargetBarriers = true;

		mCombinedSourceStages |= barrier.SourceStages;
		mCombinedDestinationStages |= barrier.DestinationStages;
	}

	void MetalBarrierHelper::Execute(id<MTLRenderCommandEncoder> renderEncoder, id<MTLComputeCommandEncoder> computeEncoder)
	{
		if(HasBarriers())
		{
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			if(renderEncoder != nil)
			{
				B3D_ASSERT(!RequiresRenderPassRestart());

				MTLBarrierScope scope = (MTLBarrierScope)0;
				if(mHasBufferBarriers)
					scope |= MTLBarrierScopeBuffers;
				if(mHasTextureBarriers)
					scope |= MTLBarrierScopeTextures;

				const GpuStageFlags vertexStages = GpuStageFlag::DrawIndirect
					| GpuStageFlag::VertexInputAttributes | GpuStageFlag::VertexInputIndices
					| GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::VertexShaderUniform;
				const GpuStageFlags fragmentStages = GpuStageFlag::FragmentShaderNonUniform
					| GpuStageFlag::FragmentShaderUniform | GpuStageFlag::EarlyFragmentTests
					| GpuStageFlag::LateFragmentTests | GpuStageFlag::ColorAttachment;

				MTLRenderStages beforeStages = (MTLRenderStages)0;
				if(mCombinedDestinationStages.IsSetAny(vertexStages))
					beforeStages |= MTLRenderStageVertex;
				if(mCombinedDestinationStages.IsSetAny(fragmentStages))
					beforeStages |= MTLRenderStageFragment;
				if(beforeStages == 0)
					beforeStages = MTLRenderStageVertex;

				// Apple-family GPUs support vertex-stage producers only. Dependencies involving a
				// fragment/tile producer or render-target scope are resolved by the command buffer
				// through an encoder boundary before Execute is called.
				[renderEncoder memoryBarrierWithScope:scope
					afterStages:MTLRenderStageVertex
					beforeStages:beforeStages];
			}
			else if(computeEncoder != nil)
			{
				// MTLBarrierScopeRenderTargets is invalid on compute encoders; attachment barriers
				// recorded while a compute encoder is open resolve as texture-scope barriers.
				MTLBarrierScope scope = (MTLBarrierScope)0;
				if(mHasBufferBarriers)
					scope |= MTLBarrierScopeBuffers;
				if(mHasTextureBarriers || mHasRenderTargetBarriers)
					scope |= MTLBarrierScopeTextures;

				[computeEncoder memoryBarrierWithScope:scope];
			}
			// else: no render/compute encoder is open, so there is no intrapass barrier to emit.
			// Explicit mode synchronizes encoder transitions with the command buffer's MTLFence.
#endif
			// Tracked mode deliberately emits no native barrier. Metal's tracked resources provide
			// the dependency; the generic tracker state below is still advanced for verification and
			// backend parity.
		}

		// Post-barrier bookkeeping runs unconditionally: Track*Usage defers hazard registrations even
		// when no native barrier was queued, and they must commit at this point (see the core
		// CommitPendingHazardRegistrations contract).
		ApplyPostBarrierTracking();
		mResourceTracker->CommitPendingHazardRegistrations();
		Clear();
	}

	void MetalBarrierHelper::Clear()
	{
		mHasBufferBarriers = false;
		mHasTextureBarriers = false;
		mHasRenderTargetBarriers = false;
		mCombinedSourceStages = GpuStageFlag::None;
		mCombinedDestinationStages = GpuStageFlag::None;

		TGpuBarrierHelper<MetalBarrierHelper>::Clear();
	}

	bool MetalBarrierHelper::HasBarriers() const
	{
		return mHasBufferBarriers || mHasTextureBarriers;
	}

	bool MetalBarrierHelper::RequiresRenderPassRestart() const
	{
		if(!HasBarriers())
			return false;

		if(mHasRenderTargetBarriers)
			return true;

		const GpuStageFlags supportedSourceStages = GpuStageFlag::DrawIndirect
			| GpuStageFlag::VertexInputAttributes | GpuStageFlag::VertexInputIndices
			| GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::VertexShaderUniform;
		const GpuStageFlags supportedDestinationStages = supportedSourceStages
			| GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::FragmentShaderUniform
			| GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests
			| GpuStageFlag::ColorAttachment;

		return !(mCombinedSourceStages & ~supportedSourceStages).IsEmpty()
			|| !(mCombinedDestinationStages & ~supportedDestinationStages).IsEmpty();
	}
} // namespace b3d::render
