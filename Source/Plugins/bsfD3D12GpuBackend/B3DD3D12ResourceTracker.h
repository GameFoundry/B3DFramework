//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	class D3D12BarrierHelper;
	struct D3D12FramebufferAttachment;

	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	extern template class TGpuResourceTracker<D3D12BarrierHelper>;

	/** Receives submission hazards for native buffer pages shared by multiple logical buffers. */
	class D3D12BufferPageSubmissionTransitionVisitor
	{
	public:
		virtual ~D3D12BufferPageSubmissionTransitionVisitor() = default;

		/** Processes the submission hazards accumulated for @p page. */
		virtual void VisitBufferPage(D3D12BufferPage& page, const GpuSubmissionTransition& transition) = 0;
	};

	/**
	 * D3D12-specific resource tracker. Logical hazards and image layouts remain in the core tracker; this subclass adds
	 * render-target integration and physical hazards for buffers sharing a pooled native page.
	 */
	class D3D12ResourceTracker : public TGpuResourceTracker<D3D12BarrierHelper>
	{
	public:
		/** Tracks one logical buffer use through core and records any conservative hazard for its shared native page. */
		void TrackBufferUsage(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper, u32 dynamicOffset = 0);

		/**
		 * Registers each attachment of a render target as used on the associated command buffer, queuing any required
		 * transitions into @p barrierHelper (execute them before the pass records its work).
		 */
		void TrackRenderTargetUsage(const D3D12FramebufferAttachment* attachments, u32 attachmentCount, RenderSurfaceMask readOnlyMask, D3D12BarrierHelper& barrierHelper);

		/** Clears framebuffer-use flags for every tracked subresource of @p image when a render pass ends. */
		void ClearRenderTargetFlagsForImage(D3D12Image* image);

		/** Clears shader-use flags for every subresource touched during the current render pass. */
		void ClearShaderFlagsForAllRenderPassImageSubresources();

		/** Resolves cross-queue write hazards for every buffer page used by this command buffer. */
		void ResolveBufferPageSubmissionTransitions(GpuQueueId destinationQueueId, D3D12BufferPageSubmissionTransitionVisitor& visitor);

		/** Clears all logical and physical resource tracking for the current recording. */
		void Clear();

	private:
		friend class D3D12BarrierHelper;

		/** Collects a physical page write performed by the next recorded GPU command. */
		void TrackBufferPageAccess(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags access);

		/** Resolves pending physical page accesses and appends their barriers to @p barrierHelper. */
		void ResolvePendingBufferPageHazards(D3D12BarrierHelper& barrierHelper);

		struct PendingBufferPageAccess
		{
			/** Creates an empty access scope for @p page. */
			explicit PendingBufferPageAccess(D3D12BufferPage* page) : Page(page) { }

			D3D12BufferPage* Page = nullptr; /**< Shared native page being accessed. */
			GpuAccessScope Scope;            /**< Combined writes performed by the next GPU command. */
		};

		// TODO - We might want to consider merging 'backing storage' into core resource tracker. If another API needs it. Otherwise we'll probably be moving away
		// from this concept and implement D3D12 tight alignment that doesn't require this backing storage.
		TInlineArray<PendingBufferPageAccess, 4> mPendingBufferPageAccesses; /**< Page writes made by the next GPU command. */
		UnorderedMap<D3D12BufferPage*, GpuResourceHazardState> mBufferPageHazards; /**< Page hazards within this command-buffer recording. */
	};

	/** @} */
} // namespace b3d::render
