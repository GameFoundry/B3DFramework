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
	 * Helper class that tracks resource usage, layout transitions, and hazard tracking for VulkanGpuCommandBuffer.
	 *
	 * This class encapsulates all the logic for:
	 * - Tracking which resources (images, buffers) are used on a command buffer
	 * - Managing image subresource states and layout transitions
	 * - Performing subresource range subdivision when needed
	 * - Validating memory hazards (read-after-write, write-after-read)
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

#if B3D_AUTOMATIC_BARRIERS
			/**
			 * Use flags when subresource is bound for any kind of operation that will require an execution or memory
			 * barrier due to a write hazard. Currently used for issuing execution/memory barriers after shader writes
			 * (not counting render pass writes, which handles barriers through subpass dependencies, or transfer operations
			 * which handle the barriers explicitly). Reset after a memory barrier is issued.
			 */
			ResourcePipelineUse WriteHazardUse;

			/**
			 * Use flags to set after running the pipeline barrier. Ensures that resource accesses after the barrier
			 * trigger barriers on their next use.
			 */
			ResourcePipelineUse NewWriteHazardUse;
#endif

			/**
			 * Specifies how will the subresource be used during the current render pass or dispatch call. Reset
			 * after use.
			 */
			ImageUseFlags UseFlags;

			/** Determines is the initial use of this subresource read-only. Used for better determining access flags. */
			bool InitialReadOnly = false;

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

#if B3D_HAZARD_TRACKING
			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued*/
			WriteHazardTracking* WriteHazardTracking = nullptr;
#endif
		};

		/**
		 * Constructs a resource tracker associated with the provided command buffer.
		 *
		 * @param commandBuffer		Command buffer that this tracker will be associated with. Used for callbacks
		 *							that need to interact with command buffer state (e.g., render pass state).
		 */
		VulkanResourceTracker(VulkanGpuCommandBuffer* commandBuffer);

		/** Creates a new tracking state for the buffer (if this is the first time the buffer has been used on the command buffer), or returns existing tracking state. */
		BufferTrackingState& GetOrCreateBufferTrackingState(VulkanBuffer* buffer);

		/** Lets the tracker know that the provided buffer resource has been queued the associated command buffer. */
		void TrackBufferUsage(BufferTrackingState& bufferTrackingState, GpuResourceUseFlags useFlags, GpuAccessFlags access);

		/** Same as the other overload, except it automatically creates/finds the BufferTrackingState object. */
		void TrackBufferUsage(VulkanBuffer* buffer, GpuResourceUseFlags useFlags, GpuAccessFlags access);

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
#endif

		// TODO - These are temporarily exposed during the process of moving towards the tracker
		TDenseMap<VulkanResource*, BufferTrackingState>& GetBuffers() { return mBuffers; }

	private:
		friend class VulkanGpuCommandBuffer;

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

		/** Set of global subresource indices that are currently bound for shader access. */
		Set<u32> mShaderBoundSubresourceInfos;

		/** Maps images to their global image tracking state index if they have queued layout transitions. */
		UnorderedMap<VulkanImage*, u32> mQueuedLayoutTransitions;

#if B3D_HAZARD_TRACKING
		/** Pool allocator for WriteHazardTracking structures. */
		PoolAlloc<sizeof(WriteHazardTracking)> mWriteHazardPool;
#endif
	};

	/** @} */
} // namespace b3d::render
