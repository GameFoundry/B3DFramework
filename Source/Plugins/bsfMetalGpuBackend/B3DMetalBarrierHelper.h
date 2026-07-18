//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuBarrierHelper.h"

namespace b3d::render
{
	class MetalResourceTracker;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Helper class for building and issuing Metal memory barriers.
	 *
	 * The backend-independent machinery (hazard analysis, subresource subdivision, post-barrier
	 * tracker updates) lives in the TGpuBarrierHelper base. This class accumulates the Metal-native
	 * side: which @c MTLBarrierScope categories need a barrier and the union of engine pipeline
	 * stages involved, then emits at most one @c memoryBarrierWithScope: on the currently open
	 * encoder in @c Execute.
	 *
	 * Design notes:
	 *  - Barriers are scope-based rather than per-resource: Metal's @c memoryBarrierWithScope:
	 *    synchronizes whole resource categories, which is coarse but can never under-synchronize.
	 *    A per-resource @c memoryBarrierWithResources: refinement requires native-handle accessors
	 *    on the tracked-resource layer. TODO(metal-tracking): refine once MetalBuffer/MetalImage
	 *    expose their id<MTL...> handles.
	 *  - Inter-pass synchronization belongs to @c MetalGpuCommandBuffer: explicit mode updates and
	 *    waits on an @c MTLFence at encoder transitions. This helper owns the complementary
	 *    intra-pass memory barriers. Tracked mode advances tracker bookkeeping without emitting
	 *    redundant native barriers.
	 *  - Image "layout transitions" are pure bookkeeping on Metal (there is no VkImageLayout
	 *    analogue); the base's layout tracking still runs so the core tracker state stays coherent.
	 *
	 * Typical usage (from MetalGpuCommandBuffer):
	 * @code
	 * mResourceTracker.TrackBufferUsage(buffer, usage, access, mBarrierHelper);
	 * mBarrierHelper.Execute(renderEncoder, computeEncoder); // before recording the dependent command
	 * @endcode
	 */
	class MetalBarrierHelper : public TGpuBarrierHelper<MetalBarrierHelper>
	{
	public:
		/**
		 * Constructs a barrier helper associated with the provided resource tracker.
		 *
		 * @param resourceTracker	Object responsible for tracking all resource usages on a command buffer. Used for
		 *							determining current object state, and notified with new state when barriers and
		 *							layout transitions are executed.
		 */
		MetalBarrierHelper(MetalResourceTracker* resourceTracker);

#ifdef __OBJC__
		/**
		 * Emits the accumulated barriers on the currently open encoder, then runs the post-barrier
		 * tracker updates (ApplyPostBarrierTracking, CommitPendingHazardRegistrations) and clears the
		 * accumulated state. Always call this after a batch of Track*Usage / Add*Barrier calls and
		 * before recording the dependent commands — even when HasBarriers() is false — so deferred
		 * hazard registrations commit at the right point.
		 *
		 * @param renderEncoder		Currently open render encoder, or nil.
		 * @param computeEncoder	Currently open compute encoder, or nil. Ignored when @p renderEncoder is set.
		 *							When both are nil (no encoder, or a blit encoder is open) no intrapass barrier
		 *							is emitted. Tracked mode delegates inter-pass hazards to Metal; explicit mode
		 *							uses the command buffer's MTLFence and queue-event path.
		 */
		void Execute(id<MTLRenderCommandEncoder> renderEncoder, id<MTLComputeCommandEncoder> computeEncoder);
#endif

		/**
		 * Clears all accumulated barriers without executing them. Useful if you need to reset the
		 * helper state without issuing barriers.
		 */
		void Clear();

		/** Returns true if there are any barriers accumulated and ready to execute. */
		bool HasBarriers() const;

		/**
		 * Returns true when the pending render-encoder dependency cannot be represented by an
		 * Apple-family intrapass memory barrier and therefore requires a render-pass boundary.
		 */
		bool RequiresRenderPassRestart() const;

	private:
		friend class TGpuBarrierHelper<MetalBarrierHelper>;

		/** CRTP hook: accumulates the buffer barrier's scope + stage union. Called by the shared low-level path. */
		void RecordBufferBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& barrier);

		/**
		 * CRTP hook: accumulates the image barrier's scope + stage union. Metal performs no native layout
		 * transitions, so unlike the Vulkan hook @p oldLayout is taken by value and never reconciled —
		 * the base's layout bookkeeping proceeds with the tracked value unchanged.
		 */
		void RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange,
			const GpuHazardStageAndAccess& barrier, GpuImageLayout oldLayout, GpuImageLayout newLayout);

		// Engine-typed native accumulation (no Metal types here so the header stays includable from
		// plain C++ TUs). Converted to MTLBarrierScope / MTLRenderStages inside Execute.
		bool mHasBufferBarriers = false;
		bool mHasTextureBarriers = false;
		bool mHasRenderTargetBarriers = false;

		// Union of engine stages seen across the accumulated barriers. Execute maps the subset
		// supported by Apple-family render barriers; unsupported producer dependencies are resolved
		// by a render-pass boundary in MetalGpuCommandBuffer.
		GpuStageFlags mCombinedSourceStages = GpuStageFlag::None;
		GpuStageFlags mCombinedDestinationStages = GpuStageFlag::None;
	};

	extern template class TGpuBarrierHelper<MetalBarrierHelper>;

	/** @} */
} // namespace b3d::render
