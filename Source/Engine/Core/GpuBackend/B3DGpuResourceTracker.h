//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuHazardTracking.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Allocators/B3DPoolAlloc.h"
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
			/** Whether this resource has been submitted to the GPU queue. */
			bool Used;

			/** Access flags indicating how the resource is being accessed (read/write). */
			GpuAccessFlags Flags;
		};

		/** Contains information about a single GPU buffer resource bound/used on a command buffer. */
		struct GpuBufferTrackingState
		{
			/** Information about resource usage and submission state. */
			GpuResourceUseHandle UseHandle;

			/** Flags indicating how the buffer is being used (shader access, transfer, etc.). */
			GpuResourceUseFlags UseFlags;

			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued. */
			GpuHazardTracking* WriteHazardTracking = nullptr;

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Suballocation indices that are bound in this tracking state. Typically 1-2. */
			TInlineArray<u32, 2> BoundSuballocationIndices;
#endif
		};

		/** Contains information about a single GPU image resource bound/used on a command buffer. */
		struct GpuImageTrackingState
		{
			/** Information about resource usage and submission state. */
			GpuResourceUseHandle UseHandle;

			/** Index of the first subresource tracking state in the global subresource tracking array. */
			u32 FirstSubresourceInfoIndex;

			/** Number of consecutive subresource tracking states belonging to this image. */
			u32 SubresourceInfoCount;
		};

		/** Contains information about a range of GPU image sub-resources bound/used on a command buffer. */
		struct GpuImageSubresourceTrackingState
		{
			/** The subresource range (mip levels and array layers) covered by this tracking state. */
			GpuTextureSubresourceRange Range;

			// Storing stage & access flags separately per use category so they can be cleared independantly when that use
			// ends (e.g. image unbound as FB attachment, or memory barrier executed)

			/** Use flags when subresource is bound for shader reads or writes. Reset after resource is unbound. */
			GpuAccessFlags ShaderUse;

			/** Use flags when subresource is bound as a framebuffer attachment. Reset after resource is unbound. */
			GpuAccessFlags FramebufferUse;

			/** Specifies how will the subresource be accessed during the current render pass or dispatch call. Unlike accesses in *Use structs, this one is not reset after render pass. */
			GpuAccessFlags Access;

			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued. */
			GpuHazardTracking* WriteHazardTracking = nullptr;

			// Only relevant for layout transitions
			/**
			 * Layout transition performed during the command buffer submit. This will be the initial layout of the
			 * image when the command buffer starts executing.
			 */
			GpuImageLayout InitialLayout;

			/**
			 * Layout the image is currently in. This will be the initial layout if no other transition was performed, or
			 * layout resulting from the last performed transition.
			 */
			GpuImageLayout CurrentLayout;

			/**
			 * Stores the layout that the image needs to be before being used in the current render pass or dispatch call.
			 * Equal to CurrentLayout if no transition is needed. Updated after every render pass or dispatch call.
			 */
			GpuImageLayout RequiredLayout;

			/**
			 * Layout the image will have after the render pass executes, taking account automatic transitions render pass
			 * does on its attachments. Only relevant for framebuffer attachments. Ignored if render pass doesn't execute.
			 */
			GpuImageLayout RenderPassLayout;
		};

		/**
		 * Tracker for all resources used on a single command buffer. Keeps bound resources alive while they are bound on the 
		 * command buffer, keeps track of necessary barriers and layout transitions that need to be issued.
		 *
		 * @tparam	TBarrierHelper	Backend-specific barrier helper. Must expose AddBufferBarrier / AddSubresourceBarrier and a
		 *							nested BarrierTrackingInfo type, which will be used for accumulating required barriers.
		 *							After barriers are issued, the barrier helper must notify the resource tracker via the 
		 *							Update*TrackingAfterBarrier() methods and finally call CommitPendingHazardRegistrations().
		 */
		template<class TBarrierHelper>
		class TGpuResourceTracker
		{
		public:
			/**
			 * Lets the tracker know that the provided buffer resource will be queued on the associated command buffer. Call this before the buffer is used, with
			 * the appropriate usage of how is it about to be used. Execute the barriers queued in @p barrierHelper before use.
			 *
			 * @param	buffer				Buffer to track.
			 * @param	useFlags			Categorizes how the buffer will be used (shader access, vertex input, etc.), and on which stages.
			 * @param	accessFlags			Access flags specifying how the buffer will be accessed (read/write).
			 * @param	barrierHelper		If there are any necessary memory barriers before the buffer can be used they will be recorded into the provided object.
			 * @param	dynamicOffset		Byte offset into the buffer (e.g., for dynamic uniform buffers). Used to calculate suballocation index for tracking in debug builds.
			 */
			void TrackBufferUsage(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, u32 dynamicOffset = 0);

			/**
			 * Lets the tracker know that the provided image resource will be queued on the associated command buffer. Call this before the image is used, with
			 * the appropriate usage of how is it about to be used. Execute the barriers queued in @p barrierHelper before use.
			 * Use this only for images bound as shader parameters - for attachments use the backend framebuffer tracking instead.
			 *
			 * @param	image				Image to track.
			 * @param	subresourceRange	Subresource range of the image to track.
			 * @param	layout				Expected layout the image should be during use.
			 * @param	finalLayout			Layout the image will be in after render pass completes (relevant only for framebuffer attachments).
			 * @param	useFlags			Categorizes how the image be used (shader access, color attachment, depth attachment, etc.), and on which stages.
			 * @param	accessFlags			Access flags specifying how the image will be accessed (read/write).
			 * @param	barrierHelper		If there are any necessary layout transitions or memory barriers before the buffer can be used they will be recorded into the provided object.
			 */
			void TrackImageUsage(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper);

			/** Lets the tracker know that the provided swap chain will be queued on the associated command buffer. */
			void TrackSwapChainUsage(IGpuSwapChainResource* swapChain);

			/**
			 * Lets the tracker know that the provided resource will be queued on the associated command buffer.
			 * If a resource is an image, buffer, swap chain or framebuffer use the more specific Track*Use() overload.
			 */
			void TrackResourceUsage(IGpuResource* resource, GpuAccessFlags access);

			/**
			 * Iterates over all subresource tracking states that overlap with the provided subresource range. The provided callback is invoked for each overlapping subresource.
			 * If a subresource state partially overlaps the provided range, the system will subdivide existing state so it can return only the fully overlapping ranges.
			 * If a tracking state for a range doesn't exist, it will be created.
			 *
			 * @param	image							Image whose subresources to iterate.
			 * @param	subresourceRange				Subresource range to find overlaps for.
			 * @param	fnDoOnOverlappingSubresource	Callback invoked for each overlapping subresource. Receives global subresource index and user data pointer.
			 * @param	userData						Optional user data passed to the callback.
			 */
			void IterateAndCreateOverlappingImageSubresourceTrackingState(IGpuImageResource* image, GpuTextureSubresourceRange subresourceRange, void(*fnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData = nullptr);

			/** Finds a read-only buffer tracking state for the specified buffer. */
			const GpuBufferTrackingState* FindBufferTrackingState(IGpuBufferResource* buffer) const;

			/** Finds the tracking state for the specified image, or returns nullptr if not found. */
			const GpuImageTrackingState* FindImageTrackingState(IGpuImageResource* image) const;

			/** Returns a read-only view of all subresource tracking states for the specified image. */
			TArrayView<const GpuImageSubresourceTrackingState> GetSubresourceTrackingStatesForImage(IGpuImageResource* image) const;

			/** Returns a mutable view of all subresource tracking states for the specified image. */
			TArrayView<GpuImageSubresourceTrackingState> GetSubresourceTrackingStatesForImage(IGpuImageResource* image);

			/** Returns the subresource tracking state at the specified global index. */
			const GpuImageSubresourceTrackingState& GetSubresourceTrackingStateAtIndex(u32 globalSubresourceIndex) { return mSubresourceTrackingState[globalSubresourceIndex]; }

			/** Finds a read-only subresource tracking state for the specified face and mip level of the provided image. */
			const GpuImageSubresourceTrackingState& GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip) const;

			/** Finds a read-only subresource tracking state for the specified face and mip level of the provided image. */
			const GpuImageSubresourceTrackingState* FindSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip) const;

			/** Notifies all tracked resources that the command buffer has submitted to a GPU queue. */
			void NotifyUsed(GpuQueueId queueId);

			/** Notifies all tracked resources that the command buffer has finished executing on a GPU queue. */
			void NotifyDone(GpuQueueId queueId);

			/** Notifies all tracked resources that they have been unbound from the command buffer. Usually called if command buffer is destroyed or reset before being submitted. */
			void NotifyUnbound();

			/**
			 * Clears all tracked resources and resets the tracker to initial state.
			 * Should be called when the command buffer is reset.
			 */
			void Clear();

			/** Updates image layout tracking for a single image subresource after a barrier has been issued. */
			void UpdateImageLayoutTrackingAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout oldLayout, GpuImageLayout newLayout);

			/** Updates write hazard tracking for a single buffer after a barrier has been issued. */
			void UpdateWriteHazardTrackingAfterBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& barrier);

			/** Updates write hazard tracking for a single image after a barrier has been issued. */
			void UpdateWriteHazardTrackingAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuHazardStageAndAccess& barrier);

			/**
			 * Applies all read/write hazard registrations deferred by TrackBufferUsage / TrackSubresourceUsage. Call after any
			 * pending barriers have been issued, or after a recording scope that cannot contain barriers has ended. Deferring the
			 * registration ensures a newly recorded write remains unsafe for the next access instead of being clobbered by the
			 * AddSafeAccess that resolves a previous write.
			 */
			void CommitPendingHazardRegistrations();

			/** Returns the internal map of all tracked buffers and their tracking states. */
			TDenseMap<IGpuBufferResource*, GpuBufferTrackingState>& GetBuffers() { return mBuffers; }

			/** Returns the internal map of all tracked images to their tracking state indices. */
			TDenseMap<IGpuImageResource*, u32>& GetImages() { return mImages; }

		private:
			/** Creates a new tracking state for the buffer (if this is the first time the buffer has been used on the command buffer), or returns existing tracking state. */
			GpuBufferTrackingState& GetOrCreateBufferTrackingState(IGpuBufferResource* buffer);

			/** Creates a new tracking state for the image (if this is the first time the image has been used on the command buffer), or returns existing tracking state. */
			GpuImageTrackingState& GetOrCreateImageTrackingState(IGpuImageResource* image);

			/** Retrieves the tracking state for the specified image. The image must have been previously tracked. */
			const GpuImageTrackingState& GetImageTrackingState(IGpuImageResource* image) const;

			/** Retrieves the tracking state for the specified image. The image must have been previously tracked. */
			GpuImageTrackingState& GetImageTrackingState(IGpuImageResource* image);

			/** Finds the tracking state index for the specified image, or returns ~0u if not found. */
			u32 FindImageTrackingStateIndex(IGpuImageResource* image) const;

			/**
			 * Private overload of TrackBufferUsage that operates on an existing GpuBufferTrackingState.
			 * Lets the tracker know that the provided buffer resource will be queued on the associated command buffer.
			 * Handles suballocation tracking in debug builds.
			 *
			 * @param	dynamicOffset		Byte offset into the buffer. Used to calculate suballocation index for tracking.
			 */
			void TrackBufferUsage(IGpuBufferResource* buffer, GpuBufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access, TBarrierHelper& barrierHelper, u32 dynamicOffset = 0);

			/**
			 * Lets the tracker know that the provided image subresource range resource will be queued the associated command buffer. This does bulk of the work to determine necessary layout transitions
			 * and barriers based on previous subresource usage.
			 */
			// TODO - Refactor this signature, try to clean it up once we have explicit layout transitions
			void TrackSubresourceUsage(IGpuImageResource* image, u32 globalSubresourceIndex, GpuImageLayout layout, GpuImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper);

			/**
			 * Private overload of IterateAndCreateOverlappingImageSubresourceTrackingState that operates on an existing GpuImageTrackingState.
			 * Iterates over all subresource tracking states that overlap with the provided subresource range. The provided callback is invoked for each overlapping subresource.
			 * If a subresource state partially overlaps the provided range, the system will subdivide existing state so it can return only the fully overlapping ranges.
			 * If a tracking state for a range doesn't exist, it will be created.
			 *
			 * @param	imageTrackingState				Existing image tracking state to operate on.
			 * @param	image							Image whose subresources to iterate.
			 * @param	subresourceRange				Subresource range to find overlaps for.
			 * @param	fnDoOnOverlappingSubresource	Callback invoked for each overlapping subresource. Receives global subresource index and user data pointer.
			 * @param	userData						Optional user data passed to the callback.
			 */
			void IterateAndCreateOverlappingImageSubresourceTrackingState(GpuImageTrackingState& imageTrackingState, const IGpuImageResource& image, GpuTextureSubresourceRange subresourceRange, void(*fnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData = nullptr);

			/** Registers a new resource range using the provided parameters to initialize it. */
			u32 AddSubresourceTrackingState(const GpuTextureSubresourceRange& range);

			/**
			 * Creates a copy of an existing subresource with a new range.
			 *
			 * @param	copyFromIndex				Global index of the subresource to copy from.
			 * @param	newRange					The new subresource range to assign to the copy.
			 * @return								Global index of the newly created subresource.
			 */
			u32 CopySubresourceTrackingStateWithNewRange(u32 copyFromIndex, const GpuTextureSubresourceRange& newRange);

		protected:
			/** Finds a subresource tracking state for the specified face and mip level of the provided image. */
			GpuImageSubresourceTrackingState& GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip);

			/** Maps images to their tracking state index in mImageTrackingState. */
			TDenseMap<IGpuImageResource*, u32> mImages;

			/** Maps buffers to their tracking state. */
			TDenseMap<IGpuBufferResource*, GpuBufferTrackingState> mBuffers;

			/** All generic resources tracked by this command buffer. */
			TDenseMap<IGpuResource*, GpuResourceUseHandle> mResources;

			/** Maps swap chains to their use handles. */
			TDenseMap<IGpuSwapChainResource*, GpuResourceUseHandle> mSwapChains;

			/** Storage for all image tracking states. Index corresponds to values in mImages. */
			Vector<GpuImageTrackingState> mImageTrackingState;

			/** Storage for all image subresource tracking states. GpuImageTrackingState references ranges within this storage. */
			Vector<GpuImageSubresourceTrackingState> mSubresourceTrackingState;

			/** Set of global subresource indices that are used on the current render pass. */
			Set<u32> mRenderPassSubresources;

			/** Pool allocator for GpuHazardTracking structures. */
			PoolAlloc<sizeof(GpuHazardTracking), 512, alignof(GpuHazardTracking)> mHazardTrackingPool;

			/** A read/write hazard registration deferred until the pending barriers have been issued. */
			struct PendingHazardRegistration
			{
				GpuHazardTracking* Tracking;
				GpuStageFlags AccessStageFlags;
				GpuAccessFlags Access;
			};

			/** Read/write hazard registrations deferred until the pending barriers are issued (see CommitPendingHazardRegistrations). */
			Vector<PendingHazardRegistration> mPendingHazardRegistrations;

		};

		/** @} */
	} // namespace render
} // namespace b3d
