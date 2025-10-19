//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "Allocators/B3DPoolAlloc.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "Utility/B3DDenseMap.h"

#define B3D_HAZARD_TRACKING B3D_BUILD_TYPE == B3D_BUILD_TYPE_DEVELOPMENT

namespace b3d::render
{
	class VulkanRenderPass;
	class VulkanGpuCommandBuffer;
	class VulkanImage;
	class VulkanBuffer;
	class VulkanFramebuffer;
	class VulkanSwapChain;

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Bits that represent different ways an image subresource can be used. */
	enum class ImageUseFlagBits // TODO - Remove these in favor of GpuResourceUsageFlags
	{
		None = 0,
		Shader = 1 << 0,
		Framebuffer = 1 << 1,
		Transfer = 1 << 2
	};

	typedef Flags<ImageUseFlagBits> ImageUseFlags;
	B3D_FLAGS_OPERATORS(ImageUseFlagBits)

#if B3D_HAZARD_TRACKING
	/** Keeps track on which pipelines was a resource written/read, and on which pipelines may it be safely accessed from. */
	struct WriteHazardPipelineTracking
	{
		static constexpr u32 kPipelineStageCount = 16;

		static constexpr VkPipelineStageFlags kAllPipelines = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;

		/** For each pipeline stage, stores in which pipelines is it safe to access the pipeline. */
		std::array<VkPipelineStageFlags, kPipelineStageCount> SafeAccess;

		WriteHazardPipelineTracking();

		/** Clears safe access for all provided pipeline stages. */
		void ClearStageSafeAccess(VkPipelineStageFlags stages);

		/**
		 * Adds safe access for all provided pipeline stages.
		 *
		 * @param	sourceStages		One or multiple stages to add the safe access to.
		 * @param	destinationStages	Stages to register as being safe to access from.
		 */
		void AddStageSafeAccess(VkPipelineStageFlags sourceStages, VkPipelineStageFlags destinationStages);

		/** Checks is it safe to access the resource in all the provided pipeline stages. */
		bool IsAccessSafe(VkPipelineStageFlags stages) const;

		/** Writes a descriptive error message when access is unsafe. */
		void LogUnsafeAccess(VkPipelineStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const;
	};

	/** Tracking that is used for validation when memory barriers need to be issued. */
	struct WriteHazardTracking
	{
		GpuAccessFlags Access; /**< Has the buffer been read or written so far. */

		/** Keeps track of all pipeline stages that the resource was read from, and which of those stages can be safely accessed by a write operation (and on which stage). */
		WriteHazardPipelineTracking ReadAccessStages;

		/** Keeps track of all pipeline stages that the resource was written to, and which of those stages can be safely accessed by a read or write operation (and on which stage). */
		WriteHazardPipelineTracking WriteAccessStages;
	};
#endif

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
			bool Used;
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
			ResourceUseHandle UseHandle;
			GpuResourceUseFlags UseFlags;

#if B3D_HAZARD_TRACKING
			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued*/
			WriteHazardTracking* WriteHazardTracking = nullptr;
#endif
		};

		/** Contains information about a single Vulkan image resource bound/used on this command buffer. */
		struct ImageTrackingState
		{
			ResourceUseHandle UseHandle;

			u32 FirstSubresourceInfoIndex;
			u32 SubresourceInfoCount;
		};

		/** Contains information about a range of Vulkan image sub-resources bound/used on this command buffer. */
		struct ImageSubresourceTrackingState
		{
			VkImageSubresourceRange Range;

			// Storing stage & access flags separately per use category so they can be cleared independantly when that use
			// ends (e.g. image unbound as FB attachment, or memory barrier executed)

			/** Use flags when subresource is bound for shader reads or writes. Reset after resource is unbound. */
			ResourcePipelineUse ShaderUse;

			/** Use flags when subresource is bound as a framebuffer attachment. Reset after resource is unbound. */
			ResourcePipelineUse FramebufferUse;

			/** Use flags when subresource is bound for a transfer operation. Currently unused. */
			ResourcePipelineUse TransferUse;

			/**
			 * Specifies how will the subresource be used during the current render pass or dispatch call. Reset
			 * after use.
			 */
			ImageUseFlags UseFlags;

			/** Specifies how will the subresource be accessed during the current render pass or dispatch call. Unlike accesses in *Use structs, this one is not reset after render pass. */
			GpuAccessFlags Access;

			/** Determines is the initial use of this subresource read-only. Used for better determining access flags. */
			bool InitialReadOnly = false;

#if B3D_HAZARD_TRACKING
			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued*/
			WriteHazardTracking* WriteHazardTracking = nullptr;
#endif

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

		/** Lets the tracker know that the provided buffer resource has been queued on the associated command buffer. */
		void TrackBufferUsage(BufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access);

		/** Same as the other overload, except it automatically creates/finds the BufferTrackingState object. */
		void TrackBufferUsage(VulkanBuffer* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags access);

		// TODO - Doc
		// Note: Use TrackframeBufferUse for framebuffer attachment images
		void TrackImageUsage(VulkanImage* image, VkImageSubresourceRange subresourceRange, ImageUseFlagBits use, VkImageLayout layout, VkImageLayout finalLayout, GpuAccessFlags access, VkPipelineStageFlags stages);
		void IterateAndCreateOverlappingImageSubresourceTrackingState(VulkanImage* image, VkImageSubresourceRange subresourceRange, void(*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData = nullptr);
		void ClearFramebufferFlagsForImage(VulkanImage* image);
		void ClearShaderFlagsForAllRenderPassImageSubresources();
		u32 FindImageTrackingStateIndex(VulkanImage* image) const;
		const ImageTrackingState* FindImageTrackingState(VulkanImage* image) const;
		TArrayView<const ImageSubresourceTrackingState> GetSubresourceTrackingStatesForImage(VulkanImage* image) const;
		TArrayView<ImageSubresourceTrackingState> GetSubresourceTrackingStatesForImage(VulkanImage* image);
		const ImageSubresourceTrackingState& GetSubresourceTrackingStateAtIndex(u32 globalSubresourceIndex) { return mSubresourceTrackingState[globalSubresourceIndex]; }
		void PopulateAndResetLayoutTransitions(TArray<VkImageMemoryBarrier>& outBarriers);

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
		 * @param	explicitReadOnlyFlags		Flags that specify which aspects of the image are known to be read-only, regardless of shader use.
		 */
		VkImageLayout GetCurrentSubresourceLayout(VulkanImage* image, const VkImageSubresourceRange& range, VulkanFramebuffer* framebuffer = nullptr, u32 explicitReadOnlyFlags = 0) const;

		/**
		 * Checks the subresource state of all associated framebuffer attachments and returns a mask of those that need to be read-only during the render pass,
		 * due to the fact they are also accessed by a shader during the same render pass.
		 */
		RenderSurfaceMask GetFramebufferReadOnlyMask(VulkanFramebuffer* framebuffer, u32 explicitReadOnlyFlags) const; // TODO - Replace u32 for flags with something typed

		/**
		 * Lets the tracker know that the provided framebuffer has been queued on the associated command buffer. All associated attachment images
		 * will be tracked as well, there's no need to track them separately.
		 */
		void TrackFramebufferUse(VulkanFramebuffer* framebuffer, RenderSurfaceMask loadMask, u32 readMask);

		/** Lets the tracker know that the provided swap chain has been queued on the associated command buffer. */
		void TrackSwapChainUse(VulkanSwapChain* swapChain);

		/**
		 * Lets the tracker know that the provided resource has been queued on the associated command buffer.
		 * If a resource is an image, buffer, swap chain or framebuffer use the more specific Track*Use() overload.
		 */
		void TrackResourceUse(VulkanResource* resource, GpuAccessFlags access);

		/** Finds a subresource tracking state for the specified face and mip level of the provided image. */
		ImageSubresourceTrackingState& FindSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip);
		const ImageSubresourceTrackingState& FindSubresourceTrackingState(VulkanImage* image, u32 face, u32 mip) const;

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

#if B3D_HAZARD_TRACKING
		/** Updates write hazard tracking for a single buffer after a barrier has been issued. */
		void UpdateWriteHazardTrackingAfterBarrier(VulkanBuffer* buffer, GpuAccessFlags sourceAccess, VkPipelineStageFlags sourceStages, GpuAccessFlags destinationAccess, VkPipelineStageFlags destinationStages);

		/** Updates write hazard tracking for a single image after a barrier has been issued. */
		void UpdateWriteHazardTrackingAfterBarrier(VulkanImage* image, const VkImageSubresourceRange& range, GpuAccessFlags sourceAccess, VkPipelineStageFlags sourceStages, GpuAccessFlags destinationAccess, VkPipelineStageFlags destinationStages);
#endif

		// TODO - These are temporarily exposed during the process of moving towards the tracker
		TDenseMap<VulkanResource*, BufferTrackingState>& GetBuffers() { return mBuffers; }
		TDenseMap<VulkanResource*, u32>& GetImages() { return mImages; }
		UnorderedSet<VulkanImage*>& GetQueuedLayoutTransitions() { return mQueuedLayoutTransitions; }
		void ClearQueuedLayoutTransitions() { mQueuedLayoutTransitions.clear(); }

	private:
		friend class VulkanGpuCommandBuffer;

		/** Creates a new tracking state for the buffer (if this is the first time the buffer has been used on the command buffer), or returns existing tracking state. */
		BufferTrackingState& GetOrCreateBufferTrackingState(VulkanBuffer* buffer);

		// TODO - Doc
		ImageTrackingState& GetOrCreateImageTrackingState(VulkanImage* image);
		const ImageTrackingState& GetImageTrackingState(VulkanImage* image) const;
		ImageTrackingState& GetImageTrackingState(VulkanImage* image);
		void IterateAndCreateOverlappingImageSubresourceTrackingState(ImageTrackingState& imageTrackingState, const VulkanImage& image, VkImageSubresourceRange subresourceRange, void(*FnDoOnOverlappingSubresource)(u32 globalSubresourceIndex, void* userData), void* userData = nullptr);

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

		/** Lets the tracker know that the provided image subresource range resource has been queued the associated command buffer. */
		// TODO - Refactor this signature, try to clean it up once we have explicit layout transitions
		void TrackSubresourceUsage(VulkanImage* image, u32 globalSubresourceIndex, ImageUseFlagBits use, VkImageLayout layout, VkImageLayout finalLayout, GpuAccessFlags access, VkPipelineStageFlags stages);

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

		UnorderedSet<VulkanImage*> mQueuedLayoutTransitions;

#if B3D_HAZARD_TRACKING
		/** Pool allocator for WriteHazardTracking structures. */
		PoolAlloc<sizeof(WriteHazardTracking)> mWriteHazardPool;
#endif
	};

	/** @} */
} // namespace b3d::render
