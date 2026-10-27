//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuBarriers.h"
#include "GpuBackend/B3DGpuFramebuffer.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Utility/B3DDenseMap.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** Contains information about a single resource bound/used on a command buffer. */
		struct GpuResourceUseHandle
		{
			/** Whether this resource has been submitted as an actual GPU access. Tracking-only entries remain false. */
			bool Used = false;

			/** Access flags indicating how the resource is being accessed. None keeps the resource alive without registering a read or write access. */
			GpuAccessFlags Flags;
		};

		/** Contains information about a single GPU buffer resource bound/used on a command buffer. */
		struct GpuBufferTrackingState
		{
			/** Information about resource usage and submission state. */
			GpuResourceUseHandle UseHandle;

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Suballocation indices that are bound in this tracking state. Typically 1-2. */
			TInlineArray<u32, 2> BoundSuballocationIndices;
#endif
		};

		/**
		 * Tracker for all resources used on a single command buffer. Keeps the resources alive while they are bound on the command buffer, records
		 * how the command buffer uses them once it is submitted, and tracks the attachments of the active render pass. It issues no barriers: the
		 * command buffer's caller issues every barrier its work needs.
		 */
		class B3D_EXPORT GpuResourceTracker
		{
		public:
			/**
			 * @name Render pass
			 * @{
			 */

			/**
			 * Starts tracking the attachments of a render pass, and tracks their image uses. Until EndRenderPass(), shaders may only read
			 * attachments that are read-only (see TrackShaderImageUsage()).
			 */
			void BeginRenderPass(TArrayView<const GpuRenderPassAttachmentUsage> attachments);

			/** Clears the active attachment tracking scope. */
			void EndRenderPass();

			/** Returns the attachments of the active render pass, or none outside of render passes. */
			TArrayView<const GpuRenderPassAttachmentUsage> GetRenderPassAttachments() const { return mRenderPassAttachments; }

			/** @} */

			/**
			 * @name Usage tracking
			 * @{
			 */

			/**
			 * Lets the tracker know that the provided buffer will be used on the associated command buffer. Keeps the buffer alive until the
			 * command buffer is done, and records the use in its submission.
			 *
			 * @param	buffer			Buffer to track.
			 * @param	access			How the command buffer accesses the buffer.
			 * @param	dynamicOffset	Byte offset into the buffer. Used to track suballocations in development builds.
			 */
			void TrackBufferUsage(IGpuBufferResource* buffer, GpuAccessFlags access, u32 dynamicOffset = 0);

			/**
			 * Lets the tracker know that the provided image subresource range will be used on the associated command buffer. Keeps the image and
			 * the subresources alive until the command buffer is done, and records the use in its submission. Use GpuAccessFlag::None to keep
			 * them alive without declaring a read or write.
			 */
			void TrackImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuAccessFlags access);

			/**
			 * Tracks an image used by a shader binding of a draw or dispatch. Validates that attachments of the active render pass are read-only
			 * and only read. Returns false for unsupported overlap.
			 */
			bool TrackShaderImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuAccessFlags access);

			/** Lets the tracker know that the provided swap chain will be queued on the associated command buffer. */
			void TrackSwapChainUsage(IGpuSwapChainResource* swapChain);

			/**
			 * Lets the tracker know that the provided resource will be queued on the associated command buffer.
			 * If a resource is an image, buffer or swap chain use the more specific Track*Usage() method.
			 */
			void TrackResourceUsage(IGpuResource* resource, GpuAccessFlags access);

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Marks the suballocation at @p offset as bound. The buffer's use must already be tracked. */
			void TrackBufferSuballocation(IGpuBufferResource* buffer, u32 offset);
#endif

			/** @} */

			/**
			 * @name Command buffer lifetime notifications
			 * @{
			 */

			/** Notifies all tracked resources that the command buffer has submitted to a GPU queue. */
			void NotifyUsed(GpuQueueId queueId);

			/** Notifies all tracked resources that the command buffer has finished executing on a GPU queue. */
			void NotifyDone(GpuQueueId queueId);

			/** Notifies all tracked resources that they have been unbound from the command buffer. Usually called if command buffer is destroyed or reset before being submitted. */
			void NotifyUnbound();

			/** Clears all tracked resources and resets the tracker to initial state. Should be called when the command buffer is reset. */
			void Clear();

			/** @} */

			/**
			 * @name Internal
			 * @{
			 */

			/** Finds the tracking state of the specified buffer, or returns null if the buffer is not tracked. */
			const GpuBufferTrackingState* FindBufferTrackingState(IGpuBufferResource* buffer) const;

			/** Finds the use handle of the specified image, or returns null if the image is not tracked. */
			const GpuResourceUseHandle* FindImageUseHandle(IGpuImageResource* image) const;

			/** Returns all tracked images and their use handles. */
			const TDenseMap<IGpuImageResource*, GpuResourceUseHandle>& GetImages() const { return mImages; }

			/** @} */

		private:
			/** Creates a new tracking state for the buffer (if this is the first time the buffer has been used on the command buffer), or returns existing tracking state. */
			GpuBufferTrackingState& GetOrCreateBufferTrackingState(IGpuBufferResource* buffer);

			/** Maps images to their use handles. */
			TDenseMap<IGpuImageResource*, GpuResourceUseHandle> mImages;

			/** Maps buffers to their tracking state. */
			TDenseMap<IGpuBufferResource*, GpuBufferTrackingState> mBuffers;

			/** All generic resources tracked by this command buffer. */
			TDenseMap<IGpuResource*, GpuResourceUseHandle> mResources;

			/** Maps swap chains to their use handles. */
			TDenseMap<IGpuSwapChainResource*, GpuResourceUseHandle> mSwapChains;

			/** Attachments of the active render pass. */
			TInlineArray<GpuRenderPassAttachmentUsage, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2> mRenderPassAttachments;

			/** True between BeginRenderPass() and EndRenderPass(). */
			bool mIsInRenderPass = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
