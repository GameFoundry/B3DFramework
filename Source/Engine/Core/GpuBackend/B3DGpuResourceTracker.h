//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuHazards.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuImageMetadataState.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuFramebuffer.h"
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

		/** Backend meta-data accesses whose executed hazards are registered by the backend itself. */
		enum class GpuImageTrackingFlag : u8
		{
			None = 0,
			MetadataOperation = 1 << 0
		};

		typedef Flags<GpuImageTrackingFlag, u8> GpuImageTrackingFlags;
		B3D_FLAGS_OPERATORS_EXT(GpuImageTrackingFlag, u8)

		/** Contains information about a single resource bound/used on a command buffer. */
		struct GpuResourceUseHandle
		{
			/** Whether this resource has been submitted as an actual GPU access. Tracking-only entries remain false. */
			bool Used;

			/**
			 * Access flags indicating how the resource is being accessed. None keeps the resource alive for barrier or
			 * layout tracking without registering a read or write access.
			 */
			GpuAccessFlags Flags;

			/** Stages of every access recorded for the resource in this command buffer. */
			GpuStageFlags Stages;
		};

		/** Contains information about a single GPU buffer resource bound/used on a command buffer. */
		struct GpuBufferTrackingState
		{
			/** Information about resource usage and submission state. */
			GpuResourceUseHandle UseHandle;

			/**
			 * State used to resolve read-after-write, write-after-write and write-after-read hazards.
			 * Null while every access recorded for the buffer is a resting read.
			 */
			GpuResourceHazardState* HazardState = nullptr;

			/** Behavior requested from the barrier preceding the first access in this command buffer. */
			GpuBarrierFlags SubmissionBarrierFlags;

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Suballocation indices that are bound in this tracking state. Typically 1-2. */
			TInlineArray<u32, 2> BoundSuballocationIndices;
#endif

			/**
			 * Returns true if every access recorded for the buffer is a resting read.
			 * Resting read resources have no hazard state tracking. This is the common state for read-only resources.
			 */
			bool HasOnlyRestingReads() const { return HazardState == nullptr && UseHandle.Stages != GpuStageFlag::None; }

			/** Returns true if the command buffer started a new lifetime of the buffer with an alias acquire. */
			bool IsAliasAcquired() const { return SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire); }

			/** Returns true if the command buffer can change the buffer's carried submission state. An alias acquire always does, as it starts a new lifetime. */
			bool HasSubmissionEffect() const { return IsAliasAcquired() || (HazardState != nullptr && HazardState->HasSubmissionEffect()); }
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

			/**
			 * Bounding range of the accesses recorded for the image, registered with the command buffer. Only valid while the image has no
			 * subresource tracking states (SubresourceInfoCount == 0). Once they exist it is stale, and they describe the accessed ranges instead.
			 */
			GpuTextureSubresourceRange Range;

			/**
			 * Returns true if every access recorded for the image is a resting read. Such an image has no subresource tracking states.
			 * This is the common state for read-only resources.
			 */
			bool HasOnlyRestingReads() const { return SubresourceInfoCount == 0 && UseHandle.Stages != GpuStageFlag::None; }
		};

		/** Contains information about a range of GPU image sub-resources bound/used on a command buffer. */
		struct GpuImageSubresourceTrackingState
		{
			/** The subresource range (mip levels and array layers) covered by this tracking state. */
			GpuTextureSubresourceRange Range;

			/** Epoch in which this range was last used through a shader binding. Multiple different usages in the same epoch can get merged into a more general usage. */
			u64 AccessEpoch = 0;

			/** Accesses recorded for this range on the current command buffer. */
			GpuAccessFlags Access;

			/** Behavior requested from the barrier preceding the first access in this command buffer. */
			GpuBarrierFlags SubmissionBarrierFlags;

			/** State used to resolve read-after-write, write-after-write and write-after-read hazards. */
			GpuResourceHazardState* HazardState = nullptr;

			/** Image meta-data tracking state, for backends that need it. */
			TShared<GpuImageMetadataState> MetadataState;

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

			/** True if the command buffer transitions the layout after the first access, which submission synchronizes like a write. */
			bool TransitionsLayout = false;

			/** Returns true if the command buffer started a new lifetime of the image with an alias acquire. */
			bool IsAliasAcquired() const { return SubmissionBarrierFlags.IsSet(GpuBarrierFlag::AliasAcquire); }

			/** Returns true if the command buffer can change the range's carried submission state. An alias acquire always does, as it starts a new lifetime. */
			bool HasSubmissionEffect() const { return IsAliasAcquired() || (HazardState != nullptr && HazardState->HasSubmissionEffect()); }
		};

		/**
		 * Tracker for all resources used on a single command buffer. Keeps bound resources alive while they are bound on the
		 * command buffer, keeps track of necessary barriers and layout transitions that need to be issued.
		 *
		 * Each resource can be in two states: Tracked & Resting
		 *  - Tracked resources - Resources have a hazard state object associated and full hazard tracking is being performed. Any resources being written is in this state.
		 *  - Resting resources - This is an optimization for the common case (most resources are read-only, such as sampleable textures or uniform buffers). Such resources
		 *						  are kept in a resting read state that is cheaper to track, and does not require a hazard state object. If a resting resource is written to, 
		 *						  it is promoted to tracked state. All buffers can always be in resting state, while for textures it depends (see Texture::CanRest() - 
		 *						  generally render targets or UAV textures never rest).
		 *
		 * @tparam	TDerived		Concrete backend resource tracker (CRTP self-type).
		 * @tparam	TBarrierHelper	Backend-specific barrier helper used to queue resolved native barriers.
		 *							After barriers are issued, the barrier helper must notify the resource tracker via the
		 *							Update*TrackingAfterBarrier() methods and finally call CommitPendingAccesses().
		 */
		template<class TDerived, class TBarrierHelper>
		class TGpuResourceTracker
		{
		public:
			/**
			 * Starts collecting resource usage for a render pass. The attachment list is copied into inline tracker storage,
			 * and shader reads accessing the render attachments are tracked.
			 */
			void PrepareRenderPass(TArrayView<const GpuRenderPassAttachmentUsage> attachments);

			/**
			 * Resolves attachment and shader usage collected since PrepareRenderPass(), then tracks the attachment image uses
			 * with a layout/access that supports attachment and optionally shader read operations.
			 * Execute any barriers queued in @p barrierHelper before beginning the native render pass.
			 */
			TArrayView<const GpuResolvedRenderPassAttachmentUsage> BeginRenderPass(TBarrierHelper& barrierHelper);

			/** Publishes native render-pass final layouts and clears the active attachment tracking scope. */
			void EndRenderPass();

			/**
			 * Returns the layout a shader read of the specified image range is performed in. Reads of a read-only attachment of the pending or active
			 * render pass use the attachment's layout, all other reads use @p requestedLayout.
			 */
			GpuImageLayout ResolveShaderImageLayout(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout requestedLayout) const;

			/**
			 * Lets the tracker know that the provided buffer resource will be used on the associated command buffer. Call this before the buffer is used, with
			 * the appropriate stage + access flags. Execute the barriers queued in @p barrierHelper before use.
			 *
			 * @param	buffer				Buffer to track.
			 * @param	stages				Stages at which the buffer will be accessed.
			 * @param	accessFlags			Access flags specifying how the buffer will be accessed (read/write).
			 * @param	barrierHelper		If there are any necessary memory barriers before the buffer can be used they will be recorded into the provided object.
			 * @param	dynamicOffset		Byte offset into the buffer (e.g., for dynamic uniform buffers). Used to calculate suballocation index for tracking in debug builds.
			 */
			void TrackBufferAccess(IGpuBufferResource* buffer, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, u32 dynamicOffset = 0);

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Marks the suballocation at @p offset as bound without changing resource access or queuing barriers. The buffer's access must already be tracked. */
			void TrackBufferSuballocation(IGpuBufferResource* buffer, u32 offset);
#endif

			/**
			 * Lets the tracker know that the provided image resource will be used on the associated command buffer. Call this before the image is used, with
			 * the appropriate stage + access flags. Execute the barriers queued in @p barrierHelper before use.
			 *
			 * @param	image				Image to track.
			 * @param	subresourceRange		Subresource range of the image to track.
			 * @param	layout				Expected layout the image should be during use.
			 * @param	stages				Stages at which the image will be accessed.
			 * @param	accessFlags			Access flags specifying how the image will be accessed (read/write).
			 * @param	barrierHelper		If there are any necessary layout transitions or memory barriers before the buffer can be used they will be recorded into the provided object.
			 * @param	barrierFlags			Additional behavior requested from the issued barrier.
			 * @param	trackingFlags		Additional behavior requested from the image tracking.
			 */
			void TrackImageAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, GpuBarrierFlags barrierFlags = GpuBarrierFlag::None, GpuImageTrackingFlags trackingFlags = GpuImageTrackingFlag::None);

			/**
			 * Tracks an image access made by a shader binding of a draw or dispatch, validating shader/attachment overlap and combining layouts within the pending access batch.
			 * Returns false for unsupported overlap. Use this instead of the generic TrackImageAccess if image is used within a shader of a draw or dispatch call.
			*/
			bool TrackShaderImageAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper);

			/** Re-tracks render pass attachment accesses that were invalidated via InvalidateRenderPassAttachmentAccess. */
			void TrackRenderPassAttachmentAccesses(TBarrierHelper& barrierHelper);

			/**
			 * Normally render targets are only tracked at the beginning of a render pass, but some backends require that we re-track them after certain operations (e.g., a clear operation implemented as a compute shader). 
			 * This ensures that correct barrier is issued between that operation stages and the raster stages when TrackRenderPassAttachmentAccesses is called.
			 */
			void InvalidateRenderPassAttachmentAccess(IGpuImageResource* image);

			/**
			 * Tracks an explicit buffer barrier. Its source scope is derived from previous command-buffer accesses. A barrier
			 * before the first access becomes a submission-entry requirement instead of a native command-list barrier.
			 *
			 * If @p aliasAcquire is set, the barrier starts a new lifetime of the buffer on memory that earlier resources used (see
			 * GpuBarrier::AliasAcquire). It orders the destination after @p aliasAcquire->Source.
			 */
			void TrackExplicitBufferBarrier(IGpuBufferResource* buffer, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, TBarrierHelper& barrierHelper, const GpuAliasAcquire* aliasAcquire = nullptr);

			/**
			 * Tracks an explicit image barrier. The tracker partitions @p subresourceRange and derives each source scope and
			 * layout. A barrier before the first access becomes a submission-entry requirement. 
			 * The image and affected subresources are retained without declaring a read or write.
			 *
			 * If @p aliasAcquire is set, the barrier starts a new lifetime of the image on memory that earlier resources used (see
			 * GpuBarrier::AliasAcquire). It transitions the whole image from GpuImageLayout::Undefined to @p destinationLayout,
			 * and orders the destination after @p aliasAcquire->Source.
			 */
			void TrackExplicitImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, GpuImageLayout destinationLayout, TBarrierHelper& barrierHelper, const GpuAliasAcquire* aliasAcquire = nullptr);

			/** Lets the tracker know that the provided swap chain will be queued on the associated command buffer. */
			void TrackSwapChainUsage(IGpuSwapChainResource* swapChain);

			/**
			 * Lets the tracker know that the provided resource will be queued on the associated command buffer.
			 * If a resource is an image, buffer, swap chain or framebuffer use the more specific Track*Use() overload.
			 */
			void TrackResourceUsage(IGpuResource* resource, GpuAccessFlags access);

			/**
			 * Resolves transitions required before the command buffer associated with this resource tracker can be submitted. Resources track their last access scope and queue, and if their first use on the tracker
			 * are different from that scope/queue, a transition needs to be issued. This information is only know at submission time, as during command buffer recording we do not know which command buffer
			 * will be submitted before it.
			 *
			 * Transitions are forwarded to @p visitor which does per-backend work to record the necessary barriers, and update the per-resource scopes for the next command buffer to utilize.
			 *
			 * The visitor must provide VisitBuffer(const GpuSubmissionBufferTransition&) and VisitImage(const GpuSubmissionImageTransition&). The visitor must not store any references to the transition descriptions,
			 * as they are only valid for the duration of the visitor call.
			 *
			 * @p frameIndex is the frame of the submission (see GpuSubmissionTransition::Build()).
			 *
			 * Submit thread only.
			 */
			void ResolveSubmissionTransitions(GpuQueueId destinationQueueId, u32 frameIndex, GpuSubmissionTransitionVisitor& visitor);

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

			/** Finds a read-only subresource tracking state for the specified face, mip level, and aspect of the provided image. */
			const GpuImageSubresourceTrackingState& GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip, GpuTextureAspectFlag aspect) const;

			/** Finds a read-only subresource tracking state for the specified face, mip level, and aspect of the provided image. */
			const GpuImageSubresourceTrackingState* FindSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip, GpuTextureAspectFlag aspect) const;

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

			/** Updates the hazard summary for a single buffer after a barrier has been issued. */
			void UpdateHazardStateAfterBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier);

			/** Updates the hazard summary for a single image after a barrier has been issued. */
			void UpdateHazardStateAfterBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuBarrierScope& barrier);

			/**
			 * Applies all read/write hazard registrations deferred by TrackBufferAccess / TrackSubresourceUsage and ends their
			 * access epoch, including when no accesses are pending. Call after any pending barriers have been issued, or after
			 * a recording scope with no barriers has ended.
			 */
			void CommitPendingAccesses();

			/** Identifies the current access batch. Advances whenever pending accesses are committed, including barrier-only batches; resets on Clear(). */
			u64 GetEpoch() const { return mEpoch; }

			/** Returns the internal map of all tracked buffers and their tracking states. */
			TDenseMap<IGpuBufferResource*, GpuBufferTrackingState>& GetBuffers() { return mBuffers; }

			/** Returns the internal map of all tracked buffers and their tracking states. */
			const TDenseMap<IGpuBufferResource*, GpuBufferTrackingState>& GetBuffers() const { return mBuffers; }

			/** Returns the internal map of all tracked images to their tracking state indices. */
			TDenseMap<IGpuImageResource*, u32>& GetImages() { return mImages; }

			/** Returns the internal map of all tracked images to their tracking state indices. */
			const TDenseMap<IGpuImageResource*, u32>& GetImages() const { return mImages; }

		private:
			/** Returns the instance of the resource tracker cast as the actual derived type. Useful to allow derived type to shadow (override) method implementations. */
			TDerived& GetDerived();

			enum class RenderPassTrackingPhase
			{
				Inactive,
				Preparing,
				Active
			};

			/** Attachment description plus shader usage accumulated while preparing a render pass. */
			struct PendingRenderPassAttachmentUsage
			{
				PendingRenderPassAttachmentUsage() = default;
				explicit PendingRenderPassAttachmentUsage(const GpuRenderPassAttachmentUsage& usage)
					: Usage(usage)
				{ }

				GpuRenderPassAttachmentUsage Usage;
				GpuResourceUseFlags ShaderUseFlags;
			};

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

			/** Returns the buffer's hazard state, creating it on first use. Resting reads recorded so far become tracked reads. */
			GpuResourceHazardState& GetOrCreateHazardState(GpuBufferTrackingState& bufferTrackingState);

			/** Finds the render-pass attachment overlapping @p range, or returns null. */
			const GpuResolvedRenderPassAttachmentUsage* FindRenderPassAttachment(IGpuImageResource* image, const GpuTextureSubresourceRange& range) const;

			/**
			 * Private overload of TrackBufferAccess that operates on an existing GpuBufferTrackingState.
			 * Lets the tracker know that the provided buffer resource will be queued on the associated command buffer.
			 * Handles suballocation tracking in debug builds.
			 *
			 * @param	dynamicOffset		Byte offset into the buffer. Used to calculate suballocation index for tracking.
			 */
			void TrackBufferAccess(IGpuBufferResource* buffer, GpuBufferTrackingState& bufferTrackingState, GpuStageFlags stages, GpuAccessFlags access, TBarrierHelper& barrierHelper, u32 dynamicOffset = 0);

			/**
			 * Lets the tracker know that the provided image subresource range resource will be queued the associated command buffer. This does bulk of the work to determine necessary layout transitions
			 * and barriers based on previous subresource usage.
			 */
			// TODO - Refactor this signature, try to clean it up once we have explicit layout transitions
			void TrackSubresourceUsage(IGpuImageResource* image, u32 globalSubresourceIndex, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags, TBarrierHelper& barrierHelper, GpuBarrierFlags barrierFlags, GpuImageTrackingFlags trackingFlags = GpuImageTrackingFlag::None);

			/** Records a resting read of @p image if the image has no subresource tracking states. Returns false if the access must be tracked instead. */
			bool TryTrackRestingImageRead(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags accessFlags);

			/** Registers a new resource range using the provided parameters to initialize it. */
			u32 AddSubresourceTrackingState(IGpuImageResource* image, const GpuTextureSubresourceRange& range);

			/**
			 * Creates a copy of an existing subresource with a new range.
			 *
			 * @param	copyFromIndex				Global index of the subresource to copy from.
			 * @param	newRange					The new subresource range to assign to the copy.
			 * @return								Global index of the newly created subresource.
			 */
			u32 CopySubresourceTrackingStateWithNewRange(u32 copyFromIndex, const GpuTextureSubresourceRange& newRange);

		protected:
			/**
			 * Retains the image and its affected subresources. Use GpuAccessFlag::None to retain them without declaring a read or write.
			 * @p stages declare on which stages is the image being accessed.
			 */
			void RegisterImageSubresources(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuAccessFlags accessFlags, GpuStageFlags stages);

			/** Selects the accesses executed for one submitted subresource. @p subresource holds the state the submission starts from. */
			const GpuResourceHazardState& ResolveImageSubmissionHazards(IGpuImageResource* image, const GpuImageSubresourceTrackingState& trackingState, GpuImageSubresource& subresource) { return *trackingState.HazardState; }

			/** Determines if a barrier is required for the provided destination usage/access, and if so queues a barrier in the barrier helper, to be executed before the next buffer access. */
			void QueueRequiredBufferBarrier(IGpuBufferResource* buffer, const GpuBufferTrackingState& bufferTrackingState, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, TBarrierHelper& barrierHelper);

			/** Initializes backend state of @p image after an alias acquire started its new lifetime. Called after the acquire's barrier is queued. */
			void InitializeAliasAcquiredImage(IGpuImageResource* image) { }

			/** Determines if a barrier is required for the provided destination usage/access, and if so queues a barrier in the barrier helper, to be executed before the next image subresource access. */
			void QueueRequiredImageBarrier(IGpuImageResource* image, GpuImageSubresourceTrackingState& subresourceTrackingState, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess, GpuImageLayout destinationLayout, TBarrierHelper& barrierHelper, GpuBarrierFlags barrierFlags = GpuBarrierFlag::None, GpuImageTrackingFlags trackingFlags = GpuImageTrackingFlag::None);

			/** Finds a subresource tracking state for the specified face, mip level, and aspect of the provided image. */
			GpuImageSubresourceTrackingState& GetSubresourceTrackingState(IGpuImageResource* image, u32 face, u32 mip, GpuTextureAspectFlag aspect);

			/** Creates optional backend meta-data values for a new image range. */
			TShared<GpuImageMetadataState> CreateImageMetadataState(IGpuImageResource* image, const GpuTextureSubresourceRange& range) { return nullptr; }

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

			/** Pool allocator for per-resource hazard states. */
			PoolAlloc<sizeof(GpuResourceHazardState), 512, alignof(GpuResourceHazardState)> mHazardStatePool;

			/** A read/write hazard registration deferred until the pending barriers have been issued. */
			struct PendingHazardRegistration
			{
				PendingHazardRegistration() = default;

				GpuResourceHazardState* State;
				GpuImageMetadataState* MetadataState = nullptr;
				GpuStageFlags AccessStageFlags;
				GpuAccessFlags Access;
			};

			/** Read/write hazard registrations deferred until the pending barriers are issued (see CommitPendingAccesses). */
			Vector<PendingHazardRegistration> mPendingHazardRegistrations;

			/** Attachments and shader usage being collected before the native render pass begins. */
			TInlineArray<PendingRenderPassAttachmentUsage, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2> mPendingRenderPassAttachments;

			/** Resolved attachments retained until the native render pass ends. */
			TInlineArray<GpuResolvedRenderPassAttachmentUsage, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2> mActiveRenderPassAttachments;

			/** Current lifecycle state of the render-pass attachment tracker. */
			RenderPassTrackingPhase mRenderPassTrackingPhase = RenderPassTrackingPhase::Inactive;

			u64 mEpoch = 1; /**< Incremented every time accesses are commited (usually after the barrier helper executes). */
			u32 mAttachmentsNeedingAccess = 0; /**< Indices of active attachments interrupted by internal operations. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
