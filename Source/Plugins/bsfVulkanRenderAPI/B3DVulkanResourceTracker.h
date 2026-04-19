//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "Allocators/B3DPoolAlloc.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "Utility/B3DDenseMap.h"

#define B3D_VERIFY_BARRIERS B3D_BUILD_TYPE_DEVELOPMENT // If enabled, ensures that memory barriers are properly issued

namespace b3d::render
{
	class VulkanBarrierHelper;
	class VulkanRenderPass;
	class VulkanGpuCommandBuffer;
	class VulkanImage;
	class VulkanBuffer;
	class VulkanFramebuffer;
	class VulkanSwapChain;

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Keeps track on which pipelines was a resource written/read, and on which pipelines may it be safely accessed from. */
	struct WriteHazardPipelineTracking
	{
		static constexpr u32 kPipelineStageCount = 16;

		/** For each pipeline stage, stores in which pipelines is it safe to access the pipeline. */
		std::array<VulkanAccessStageFlags, kPipelineStageCount> SafeAccess;

		WriteHazardPipelineTracking();

		/** Clears safe access for all provided pipeline stages. */
		void ClearStageSafeAccess(VulkanAccessStageFlags stages);

		/**
		 * Adds safe access for all provided pipeline stages.
		 *
		 * @param	sourceStages		One or multiple stages to add the safe access to.
		 * @param	destinationStages	Stages to register as being safe to access from.
		 */
		void AddStageSafeAccess(VulkanAccessStageFlags sourceStages, VulkanAccessStageFlags destinationStages);

		/** Checks is it safe to access the resource in all the provided pipeline stages. */
		bool IsAccessSafe(VulkanAccessStageFlags stages) const;

		/** Returns a list of all source stages that we cannot safely access data from the provided @p stages. */
		VulkanAccessStageFlags GetUnsafeAccessStages(VulkanAccessStageFlags stages) const;

		/** Writes a descriptive error message when access is unsafe. */
		void LogUnsafeAccess(VulkanAccessStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const;
	};

	/** Tracking that is used for validation when memory barriers need to be issued. */
	struct WriteHazardTracking
	{
		GpuAccessFlags Access; /**< Has the buffer been read or written so far. */

		/**
		 * Keeps track of all pipeline stages that the resource was read from, and which of those stages can be safely accessed by a write operation (and on which stage).
		 * A write following a read requires an execution barrier.
		 */
		WriteHazardPipelineTracking ExecutionBarrierTracking;

		/**
		 * Keeps track of all pipeline stages that the resource was written to, and which of those stages can be safely accessed by a read or write operation
		 * (and on which stage). Any operation following a write requires a memory barrier. Memory barrier implies an execution barrier as well.
		 */
		WriteHazardPipelineTracking MemoryBarrierTracking;

		/** Updates execution and memory barrier tracking by marking access as safe in the destination stages, from the source stages. */
		void AddSafeAccess(VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess);

#if B3D_VERIFY_BARRIERS
		/** Verifies that the access is safe from the provided stage and access type. Logs errors if not. */
		void VerifySafeAccess(VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess) const;
#endif

		// Note - Add LayoutTransitionTracking? Tracks last use of an imagine in a specific layout, and which layouts can be safely transitioned to/from without a barrier
	};

	/**
	 * Helper class that tracks all resources used on a command buffer. It is responsible for keeping those resources alive why they
	 * are bound on the command buffer, and also keep track of necessary barriers and layout transitions that need to be issued.
	 */
	class VulkanResourceTracker
	{
	public:
		/** Contains information about a single Vulkan resource bound/used on this command buffer. */
		struct ResourceUseHandle
		{
			/** Whether this resource has been submitted to the GPU queue. */
			bool Used;

			/** Access flags indicating how the resource is being accessed (read/write). */
			GpuAccessFlags Flags;
		};

		/** Describes where and how is a resource being accessed and by which stages. */
		struct ResourcePipelineUse
		{
			/** Specifies how will the subresource be accessed during the current render pass or dispatch call. */
			GpuAccessFlags Access;

			/** Stages the image is being used in during the current render pass or dispatch call. */
			VkPipelineStageFlags Stages = 0;
		};

		/** Contains information about a single Vulkan buffer resource bound/used on this command buffer. */
		struct BufferTrackingState
		{
			/** Information about resource usage and submission state. */
			ResourceUseHandle UseHandle;

			/** Flags indicating how the buffer is being used (shader access, transfer, etc.). */
			GpuResourceUseFlags UseFlags;

			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued*/
			WriteHazardTracking* WriteHazardTracking = nullptr;

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Suballocation indices that are bound in this tracking state. Typically 1-2. */
			TInlineArray<u32, 2> BoundSuballocationIndices;
#endif
		};

		/** Contains information about a single Vulkan image resource bound/used on this command buffer. */
		struct ImageTrackingState
		{
			/** Information about resource usage and submission state. */
			ResourceUseHandle UseHandle;

			/** Index of the first subresource tracking state in the global subresource tracking array. */
			u32 FirstSubresourceInfoIndex;

			/** Number of consecutive subresource tracking states belonging to this image. */
			u32 SubresourceInfoCount;
		};

		/** Contains information about a range of Vulkan image sub-resources bound/used on this command buffer. */
		struct ImageSubresourceTrackingState
		{
			/** The subresource range (mip levels and array layers) covered by this tracking state. */
			VkImageSubresourceRange Range;

			// Storing stage & access flags separately per use category so they can be cleared independantly when that use
			// ends (e.g. image unbound as FB attachment, or memory barrier executed)

			/** Use flags when subresource is bound for shader reads or writes. Reset after resource is unbound. */
			GpuAccessFlags ShaderUse;

			/** Use flags when subresource is bound as a framebuffer attachment. Reset after resource is unbound. */
			GpuAccessFlags FramebufferUse;

			/** Specifies how will the subresource be accessed during the current render pass or dispatch call. Unlike accesses in *Use structs, this one is not reset after render pass. */
			GpuAccessFlags Access;

			/** Determines is the initial use of this subresource read-only. Used for better determining access flags. */
			bool InitialReadOnly = false;

			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued. */
			WriteHazardTracking* WriteHazardTracking = nullptr;

			// Only relevant for layout transitions
			/**
			 * Layout transition performed during the command buffer submit. This will be the initial layout of the
			 * image when the command buffer starts executing.
			 */
			VkImageLayout InitialLayout;

			/**
			 * Layout the image is currently in. This will be the initial layout if no other transition was performed, or
			 * layout resulting from the last performed transition.
			 */
			VkImageLayout CurrentLayout;

			/**
			 * Stores the layout that the image needs to be before being used in the current render pass or dispatch call.
			 * Equal to CurrentLayout if no transition is needed. Updated after every render pass or dispatch call.
			 */
			VkImageLayout RequiredLayout;

			/**
			 * Layout the image will have after the render pass executes, taking account automatic transitions render pass
			 * does on its attachments. Only relevant for framebuffer attachments. Ignored if render pass doesn't execute.
			 */
			VkImageLayout RenderPassLayout;
		};

		/**
		 * Constructs a resource tracker associated with the provided command buffer.
		 *
		 * @param commandBuffer		Command buffer that this tracker will be associated with. Used for callbacks
		 *							that need to interact with command buffer state (e.g., render pass state).
		 */
		VulkanResourceTracker(VulkanGpuCommandBuffer* commandBuffer);

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
		void TrackBufferUsage(VulkanBuffer* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper, u32 dynamicOffset = 0);

		/**
		 * Lets the tracker know that the provided image resource will be queued on the associated command buffer. Call this before the image is used, with
		 * the appropriate usage of how is it about to be used. Execute the barriers queued in @p barrierHelper before use.
		 * Use this only for images bound as shader parameters - for attachments use TrackFramebufferUse() instead.
		 *
		 * @param	image				Image to track.
		 * @param	subresourceRange	Subresource range of the image to track.
		 * @param	layout				Expected layout the image should be during use.
		 * @param	finalLayout			Layout the image will be in after render pass completes (relevant only for framebuffer attachments).
		 * @param	useFlags			Categorizes how the image be used (shader access, color attachment, depth attachment, etc.), and on which stages.
		 * @param	accessFlags			Access flags specifying how the image will be accessed (read/write).
		 * @param	barrierHelper		If there are any necessary layout transitions or memory barriers before the buffer can be used they will be recorded into the provided object.
		 */
		void TrackImageUsage(VulkanImage* image, VkImageSubresourceRange subresourceRange, VkImageLayout layout, VkImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper);

		/**
		 * Lets the tracker know that the provided framebuffer will be queued on the associated command buffer. All associated attachment images
		 * will be tracked as well, there's no need to track them separately. Call this before the framebuffer is used. Execute the barriers
		 * queued in @p barrierHelper before use.
		 */
		void TrackFramebufferUsage(VulkanFramebuffer* framebuffer, RenderSurfaceMask loadMask, RenderSurfaceMask readOnlyMask, VulkanBarrierHelper& barrierHelper);

		/** Lets the tracker know that the provided swap chain will be queued on the associated command buffer. */
		void TrackSwapChainUsage(VulkanSwapChain* swapChain);

		/**
		 * Lets the tracker know that the provided resource will be queued on the associated command buffer.
		 * If a resource is an image, buffer, swap chain or framebuffer use the more specific Track*Use() overload.
		 */
		void TrackResourceUsage(VulkanResource* resource, GpuAccessFlags access);

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
		void IterateAndCreateOverlappingImageSubresourceTrackingState(VulkanImage* image, VkImageSubresourceRange subresourceRange, void(*fnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData = nullptr);

		/** Clears framebuffer-related usage flags for all subresources of the specified image. Usually called after render pass ends. */
		void ClearFramebufferFlagsForImage(VulkanImage* image);

		/** Clears shader-related usage flags for all image subresources that were used during the current render pass. Usually called after render pass ends. */
		void ClearShaderFlagsForAllRenderPassImageSubresources();

		/** Finds a read-only buffer tracking state for the specified buffer. */
		const BufferTrackingState* FindBufferTrackingState(VulkanBuffer* buffer) const;

		/** Finds the tracking state for the specified image, or returns nullptr if not found. */
		const ImageTrackingState* FindImageTrackingState(VulkanImage* image) const;

		/** Returns a read-only view of all subresource tracking states for the specified image. */
		TArrayView<const ImageSubresourceTrackingState> GetSubresourceTrackingStatesForImage(VulkanImage* image) const;

		/** Returns a mutable view of all subresource tracking states for the specified image. */
		TArrayView<ImageSubresourceTrackingState> GetSubresourceTrackingStatesForImage(VulkanImage* image);

		/** Returns the subresource tracking state at the specified global index. */
		const ImageSubresourceTrackingState& GetSubresourceTrackingStateAtIndex(u32 globalSubresourceIndex) { return mSubresourceTrackingState[globalSubresourceIndex]; }

		/** Finds a read-only subresource tracking state for the specified face and mip level of the provided image. */
		const ImageSubresourceTrackingState& GetSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip) const;

		/** Finds a read-only subresource tracking state for the specified face and mip level of the provided image. */
		const ImageSubresourceTrackingState* FindSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip) const;

		/**
		 * Returns the current layout of the specified image subresource, as seen by the associated command buffer. This is different from the
		 * global layout stored in VulkanImage itself, as it includes any transitions performed by the command buffer
		 * (at the current point in time), while the global layout is only updated after a command buffer as been submitted.
		 *
		 * @param	image						Image to lookup the layout for.
		 * @param	range						Subresource range of the image to lookup the layout for.
		 * @param	framebuffer					Optional framebuffer. If provided the method will assume we are currently executing a render pass with
		 *										the provided framebuffer, and will return the layout of the subresource after the render pass begins.
		 *										This may be different from the current layout if the image is used as a framebuffer attachment, in which
		 *										case the render pass may perform an automated layout transition when it begins.
		 * @param	explicitReadOnlyMask		Mask that specifies which attachments are forced to be read-only, regardless of shader use.
		 */
		VkImageLayout GetCurrentSubresourceLayout(VulkanImage* image, const VkImageSubresourceRange& range, VulkanFramebuffer* framebuffer = nullptr, RenderSurfaceMask explicitReadOnlyMask = RT_NONE) const;

		/**
		 * Checks the subresource state of all associated framebuffer attachments and returns a mask of those that need to be read-only during the render pass,
		 * due to the fact they are also accessed by a shader during the same render pass.
		 */
		RenderSurfaceMask GetFramebufferReadOnlyMask(VulkanFramebuffer* framebuffer, RenderSurfaceMask explicitReadOnlyMask) const;

		/** Updates the current layout of all associated framebuffer attachments to their final layouts after a render pass has executed. */
		void MoveAllFramebufferAttachmentsToFinalLayouts(VulkanFramebuffer* framebuffer);

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
		void UpdateImageLayoutTrackingAfterBarrier(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout oldLayout, VkImageLayout newLayout);

		/** Updates write hazard tracking for a single buffer after a barrier has been issued. */
		void UpdateWriteHazardTrackingAfterBarrier(VulkanBuffer* buffer, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess);

		/** Updates write hazard tracking for a single image after a barrier has been issued. */
		void UpdateWriteHazardTrackingAfterBarrier(VulkanImage* image, const VkImageSubresourceRange& range, VulkanAccessStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, VulkanAccessStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess);

		/** Returns the internal map of all tracked buffers and their tracking states. */
		TDenseMap<VulkanResource*, BufferTrackingState>& GetBuffers() { return mBuffers; }

		/** Returns the internal map of all tracked images to their tracking state indices. */
		TDenseMap<VulkanResource*, u32>& GetImages() { return mImages; }

	private:
		friend class VulkanGpuCommandBuffer;

		/** Creates a new tracking state for the buffer (if this is the first time the buffer has been used on the command buffer), or returns existing tracking state. */
		BufferTrackingState& GetOrCreateBufferTrackingState(VulkanBuffer* buffer);

		/** Creates a new tracking state for the image (if this is the first time the image has been used on the command buffer), or returns existing tracking state. */
		ImageTrackingState& GetOrCreateImageTrackingState(VulkanImage* image);

		/** Retrieves the tracking state for the specified image. The image must have been previously tracked. */
		const ImageTrackingState& GetImageTrackingState(VulkanImage* image) const;

		/** Retrieves the tracking state for the specified image. The image must have been previously tracked. */
		ImageTrackingState& GetImageTrackingState(VulkanImage* image);

		/** Finds the tracking state index for the specified image, or returns ~0u if not found. */
		u32 FindImageTrackingStateIndex(VulkanImage* image) const;

		/** Finds a subresource tracking state for the specified face and mip level of the provided image. */
		ImageSubresourceTrackingState& GetSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip);

		/**
		 * Private overload of TrackBufferUsage that operates on an existing BufferTrackingState.
		 * Lets the tracker know that the provided buffer resource will be queued on the associated command buffer.
		 * Handles suballocation tracking in debug builds.
		 *
		 * @param	dynamicOffset		Byte offset into the buffer. Used to calculate suballocation index for tracking.
		 */
		void TrackBufferUsage(VulkanBuffer* buffer, BufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access, VulkanBarrierHelper& barrierHelper, u32 dynamicOffset = 0);

		/**
		 * Lets the tracker know that the provided image subresource range resource will be queued the associated command buffer. This does bulk of the work to determine necessary layout transitions
		 * and barriers based on previous subresource usage.
		 */
		// TODO - Refactor this signature, try to clean it up once we have explicit layout transitions
		void TrackSubresourceUsage(VulkanImage* image, u32 globalSubresourceIndex, VkImageLayout layout, VkImageLayout finalLayout, GpuResourceUseFlags useFlags, GpuAccessFlags accessFlags, VulkanBarrierHelper& barrierHelper);

		/**
		 * Private overload of IterateAndCreateOverlappingImageSubresourceTrackingState that operates on an existing ImageTrackingState.
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
		void IterateAndCreateOverlappingImageSubresourceTrackingState(ImageTrackingState& imageTrackingState, const VulkanImage& image, VkImageSubresourceRange subresourceRange, void(*fnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData = nullptr);

		/** Registers a new resource range using the provided parameters to initialize it. */
		u32 AddSubresourceTrackingState(const VkImageSubresourceRange& range);


		/**
		 * Creates a copy of an existing subresource with a new range.
		 *
		 * @param	copyFromIndex				Global index of the subresource to copy from.
		 * @param	newRange					The new subresource range to assign to the copy.
		 * @return								Global index of the newly created subresource.
		 */
		u32 CopySubresourceTrackingStateWithNewRange(u32 copyFromIndex, const VkImageSubresourceRange& newRange);

		VulkanGpuCommandBuffer* mCommandBuffer;

		/** Maps images to their tracking state index in mImageTrackingState. */
		TDenseMap<VulkanResource*, u32> mImages;

		/** Maps buffers to their tracking state. */
		TDenseMap<VulkanResource*, BufferTrackingState> mBuffers;

		/** All generic resources tracked by this command buffer. */
		TDenseMap<VulkanResource*, ResourceUseHandle> mResources;

		/** Maps swap chains to their use handles. */
		TDenseMap<VulkanSwapChain*, ResourceUseHandle> mSwapChains;

		/** Storage for all image tracking states. Index corresponds to values in mImages. */
		Vector<ImageTrackingState> mImageTrackingState;

		/** Storage for all image subresource tracking states. ImageTrackingState references ranges within this storage. */
		Vector<ImageSubresourceTrackingState> mSubresourceTrackingState;

		/** Set of global subresource indices that are used on the current render pass. */
		Set<u32> mRenderPassSubresources;

		/** Pool allocator for WriteHazardTracking structures. */
		PoolAlloc<sizeof(WriteHazardTracking)> mWriteHazardPool;
	};

	/** @} */
} // namespace b3d::render
