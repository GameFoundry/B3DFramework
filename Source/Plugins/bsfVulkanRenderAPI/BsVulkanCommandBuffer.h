//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanResource.h"
#include "BsVulkanGpuPipelineState.h"

namespace bs
{
	namespace ct
	{
		class VulkanOcclusionQuery;
		class VulkanTimerQuery;
		class VulkanImage;
		class VulkanIndexBuffer;
		class VulkanVertexBuffer;

		/** @addtogroup Vulkan
		 *  @{
		 */

#define BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY BS_MAX_QUEUES_PER_TYPE * 32

// Maximum number of command buffers that another command buffer can be dependant on (via a sync mask)
#define BS_MAX_VULKAN_CB_DEPENDENCIES 2

		/** Wrapper around a Vulkan semaphore object that manages its usage and lifetime. */
		class VulkanSemaphore : public VulkanResource
		{
		public:
			VulkanSemaphore(VulkanResourceManager* owner);
			~VulkanSemaphore();

			/** Returns the internal handle to the Vulkan object. */
			VkSemaphore GetHandle() const { return mSemaphore; }

		private:
			VkSemaphore mSemaphore;
		};

		class VulkanCmdBuffer;

		/** Pool that allocates and distributes Vulkan command buffers. */
		class VulkanCmdBufferPool
		{
		public:
			VulkanCmdBufferPool(VulkanDevice& device);
			~VulkanCmdBufferPool();

			/**
			 * Attempts to find a free command buffer, or creates a new one if not found. Caller must guarantee the provided
			 * queue family is valid.
			 */
			VulkanCmdBuffer* GetBuffer(u32 queueFamily, bool secondary);

		private:
			/** Command buffer pool and related information. */
			struct PoolInfo
			{
				VkCommandPool Pool = VK_NULL_HANDLE;
				VulkanCmdBuffer* Buffers[BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY];
				u32 QueueFamily = -1;
			};

			/** Creates a new command buffer. */
			VulkanCmdBuffer* CreateBuffer(u32 queueFamily, bool secondary);

			VulkanDevice& mDevice;
			UnorderedMap<u32, PoolInfo> mPools;
			u32 mNextId = 1;
		};

		/** Determines where are the current descriptor sets bound to. */
		enum class DescriptorSetBindFlag
		{
			None = 0,
			Graphics = 1 << 0,
			Compute = 1 << 1
		};

		typedef Flags<DescriptorSetBindFlag> DescriptorSetBindFlags;
		B3D_FLAGS_OPERATORS(DescriptorSetBindFlag)

		/** Bits that represent different ways an image subresource can be used. */
		enum class ImageUseFlagBits
		{
			Shader = 1 << 0,
			Framebuffer = 1 << 1,
			Transfer = 1 << 2
		};

		typedef Flags<ImageUseFlagBits> ImageUseFlags;
		B3D_FLAGS_OPERATORS(ImageUseFlagBits)

		/** Bits that represent different ways a buffer can be used. */
		enum class BufferUseFlagBits
		{
			Generic = 1 << 0,
			Index = 1 << 1,
			Vertex = 1 << 2,
			Parameter = 1 << 3,
			Transfer = 1 << 4
		};

		typedef Flags<BufferUseFlagBits> BufferUseFlags;
		B3D_FLAGS_OPERATORS(BufferUseFlagBits)

		/**
		 * Represents a direct wrapper over an internal Vulkan command buffer. This is unlike VulkanCommandBuffer which is a
		 * higher level class, and it allows for re-use by internally using multiple low-level command buffers.
		 */
		class VulkanCmdBuffer
		{
			/** Possible states a command buffer can be in. */
			enum class State
			{
				/** Buffer is ready to be re-used. */
				Ready,
				/** Buffer is currently recording commands, but isn't recording a render pass. */
				Recording,
				/** Buffer is currently recording render pass commands. */
				RecordingRenderPass,
				/** Buffer is done recording but hasn't been submitted. */
				RecordingDone,
				/** Buffer is done recording and is currently submitted on a queue. */
				Submitted,
				/** Buffer is done executing on the device. */
				Done
			};

		public:
			VulkanCmdBuffer(VulkanDevice& device, u32 id, VkCommandPool pool, u32 queueFamily, bool secondary);
			~VulkanCmdBuffer();

			/** Returns an unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

			/** Returns the index of the queue family this command buffer is executing on. */
			u32 GetQueueFamily() const { return mQueueFamily; }

			/** Returns the index of the device this command buffer will execute on. */
			u32 GetDeviceIdx() const;

			/** Makes the command buffer ready to start recording commands. */
			void Begin();

			/** Ends command buffer command recording (as started with begin()). */
			void End();

			/**
			 * Submits the command buffer for execution.
			 *
			 * @param[in]	queue		Queue to submit the command buffer on.
			 * @param[in]	queueIdx	Index of the queue the command buffer was submitted on. Note that this may be different
			 *							from the actual VulkanQueue index since multiple command buffer queue indices can map
			 *							to the same queue.
			 * @param[in]	syncMask	Mask that controls which other command buffers does this command buffer depend upon
			 *							(if any). See description of @p syncMask parameter in RenderAPI::executeCommands().
			 */
			void Submit(VulkanQueue* queue, u32 queueIdx, u32 syncMask);

			/** Returns the handle to the internal Vulkan command buffer wrapped by this object. */
			VkCommandBuffer GetHandle() const { return mCmdBuffer; }

			/** Returns a fence that can be used for tracking when the command buffer is done executing. */
			VkFence GetFence() const { return mFence; }

			/**
			 * Returns a semaphore that may be used for synchronizing execution between command buffers executing on the same
			 * queue.
			 */
			VulkanSemaphore* GetIntraQueueSemaphore() const { return mIntraQueueSemaphore; }

			/**
			 * Returns a semaphore that may be used for synchronizing execution between command buffers executing on different
			 * queues. Note that these semaphores get used each time they are requested, and there is only a fixed number
			 * available. If all are used up, null will be returned. New semaphores are generated when allocateSemaphores()
			 * is called.
			 */
			VulkanSemaphore* RequestInterQueueSemaphore() const;

			/**
			 * Allocates a new set of semaphores that may be used for synchronizing execution between different command buffers.
			 * Releases the previously allocated semaphores, if they exist. Use getIntraQueueSemaphore() &
			 * requestInterQueueSemaphore() to retrieve latest allocated semaphores.
			 *
			 * @param[out]	semaphores	Output array to place all allocated semaphores in. The array must be of size
			 *							(BS_MAX_VULKAN_CB_DEPENDENCIES + 1).
			 */
			void AllocateSemaphores(VkSemaphore* semaphores);

			/** Sets the VulkanCommandBuffer that currently owns this command buffer. */
			void SetOwner(VulkanCommandBuffer* owner) { mOwner = owner; }

			/** Returns true if the command buffer is currently being processed by the device. */
			bool IsSubmitted() const { return mState == State::Submitted; }

			/** Returns true if the command buffer is currently recording (but not within a render pass). */
			bool IsRecording() const { return mState == State::Recording; }

			/** Returns true if the command buffer is ready to be submitted to a queue. */
			bool IsReadyForSubmit() const { return mState == State::RecordingDone; }

			/** Returns true if the command buffer is currently recording a render pass. */
			bool IsInRenderPass() const { return mState == State::RecordingRenderPass; }

			/** Returns true if the command buffer is done executing on the device. */
			bool IsDone() const { return mState == State::Done; }

			/**
			 * Checks is the command buffer still executing on the GPU. Internal state will be updated if execution finishes.
			 *
			 * @param[in]	block	If true, the system will block until the command buffer is done executing.
			 * @return				True if execution has finished (or was never submitted), false if still running.
			 */
			bool UpdateExecutionStatus(bool block);

			/**
			 * Resets the command buffer back in Ready state. Should be called when command buffer is done executing on a
			 * queue.
			 */
			void Reset();

			/**
			 * Lets the command buffer know that the provided resource has been queued on it, and will be used by the
			 * device when the command buffer is submitted. If a resource is an image or a buffer use the more specific
			 * registerResource() overload.
			 */
			void RegisterResource(VulkanResource* res, VulkanAccessFlags flags);

			/**
			 * Lets the command buffer know that the provided image will be used for shader reads or writes in a subsequent draw
			 * or dispatch call. Transfers the image to the provided layout and issues any necessary execution and memory
			 * barriers.
			 */
			void RegisterImageShader(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Lets the command buffer know that the provided image will be used as a framebuffer attachment in a subsequent
			 * draw call. Transfers the image to the provided layout and issues any necessary execution and memory barriers.
			 */
			void RegisterImageFramebuffer(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Lets the command buffer know that the provided image will be used for a transfer operation. Transfers the image
			 * to the provided layout and issues any necessary execution and memory barriers.
			 */
			void RegisterImageTransfer(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, VulkanAccessFlags access);
			/**
			 * Lets the command buffer know that the provided image resource has been queued on it, and will be used by the
			 * device when the command buffer is submitted. @p stages can be left empty for all uses except for generic and
			 * parameter buffer types.
			 */
			void RegisterBuffer(VulkanBuffer* res, BufferUseFlagBits useFlags, VulkanAccessFlags access, VkPipelineStageFlags stages = 0);

			/**
			 * Lets the command buffer know that the provided framebuffer resource has been queued on it, and will be used by
			 * the device when the command buffer is submitted.
			 */
			void RegisterResource(VulkanFramebuffer* res, RenderSurfaceMask loadMask, u32 readMask);

			/**
			 * Lets the command buffer know that the provided swap chain resource has been queued on it, and will be used by
			 * the device when the command buffer is submitted.
			 */
			void RegisterResource(VulkanSwapChain* res);

			/** Notifies the command buffer that the provided query has been queued on it. */
			void RegisterQuery(VulkanOcclusionQuery* query) { mOcclusionQueries.insert(query); }

			/** Notifies the command buffer that the provided query has been queued on it. */
			void RegisterQuery(VulkanTimerQuery* query) { mTimerQueries.insert(query); }

			/************************************************************************/
			/* 								COMMANDS	                     		*/
			/************************************************************************/

			/** Begins render pass recording. Must be called within begin()/end() calls. */
			void BeginRenderPass();

			/** Ends render pass recording (as started with BeginRenderPass(). */
			void EndRenderPass() { EndRenderPass(false); }

			/**
			 * Assigns a render target the the command buffer. This render target's framebuffer and render pass will be used
			 * when beginRenderPass() is called. Command buffer must not be currently recording a render pass.
			 */
			void SetRenderTarget(const SPtr<RenderTarget>& rt, u32 readOnlyFlags, RenderSurfaceMask loadMask);

			/** Clears the entirety currently bound render target. */
			void ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask);

			/** Clears the viewport portion of the currently bound render target. */
			void ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask);

			/** Assigns a pipeline state to use for subsequent draw commands. */
			void SetPipelineState(const SPtr<GraphicsPipelineState>& state);

			/** Assigns a pipeline state to use for subsequent dispatch commands. */
			void SetPipelineState(const SPtr<ComputePipelineState>& state);

			/** Assign GPU params to the GPU programs bound by the pipeline state. */
			void SetGpuParams(const SPtr<GpuParams>& gpuParams);

			/** Sets the current viewport which determine to which portion of the render target to render to. */
			void SetNormalizedViewportArea(const Rect2& area);

			/**
			 * Sets the scissor rectangle area which determines in which area if the viewport are the fragments allowed to be
			 * generated. Only relevant if enabled on the pipeline state.
			 */
			void SetScissorRect(const Rect2I& area);

			/** Sets a stencil reference value that will be used for comparisons in stencil operations, if enabled. */
			void SetStencilRef(u32 value);

			/** Changes how are primitives interpreted as during rendering. */
			void SetDrawOp(DrawOperationType drawOp);

			/** Sets one or multiple vertex buffers that will be used for subsequent draw() or drawIndexed() calls. */
			void SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers);

			/** Sets an index buffer that will be used for subsequent drawIndexed() calls. */
			void SetIndexBuffer(const SPtr<IndexBuffer>& buffer);

			/** Sets a declaration that determines how are vertex buffer contents interpreted. */
			void SetVertexDeclaration(const SPtr<VertexDeclaration>& decl);

			/** Executes a draw command using the currently bound graphics pipeline, vertex buffer and render target. */
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount);

			/** Executes a draw command using the currently bound graphics pipeline, index & vertex buffer and render target. */
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 instanceCount);

			/** Executes a dispatch command using the currently bound compute pipeline. */
			void Dispatch(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ);

			/**
			 * Registers a command that signals the event when executed. Will be delayed until the end of the current
			 * render pass, if any.
			 */
			void SetEvent(VulkanEvent* event);

			/**
			 * Registers a command that resets the query. The command will be delayed until the next submit() if a render
			 * pass is currently in progress, but is guaranteed to execute before this command buffer is submitted.
			 */
			void ResetQuery(VulkanQuery* query);

			/**
			 * Issues a pipeline barrier on the provided buffer. See vkCmdPipelineBarrier in Vulkan spec. for usage
			 * information.
			 */
			void MemoryBarrier(VkBuffer buffer, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

			/**
			 * Copies the contents of the source buffer to the destination buffer. Caller must ensure the provided
			 * offsets and length are within valid bounds of both buffers.
			 *
			 * @param	source				Source buffer to copy from.
			 * @param	destination			Destination buffer to copy to.
			 * @param	sourceOffset		Offset into the source buffer, from which to start copying, in bytes.
			 * @param	destinationOffset	Offset into the destination buffer, at which to place the copied data, in bytes.
			 * @param	length				Size of the data to copy, in bytes.
			 */
			void CopyBufferToBuffer(VulkanBuffer* source, VulkanBuffer* destination, VkDeviceSize sourceOffset, VkDeviceSize destinationOffset, VkDeviceSize length);

			/**
			 * Copies the contents of the source buffer to the destination image subresource. Caller must ensure the
			 * provided extents are within valid bounds of the image and that the provided buffer is large enough.
			 *
			 * @param	source				Source buffer to copy from.
			 * @param	destination			Destination image to copy to.
			 * @param	region				Region of the image to copy to.
			 * @param	subresourceRange	Subresource(s) of the image to copy to.
			 * @param	layout				Current layout of the image subresources in the provided range.
			 * @param	rowPitch			Determines how many pixels to advance when moving to a new row in the source buffer.
			 * @param	sliceHeight			Determines how many pixels to advance when moving to a new slice in the source buffer.
			 */
			void CopyBufferToImage(VulkanBuffer* source, VulkanImage* destination, const VkExtent3D& region, const VkImageSubresourceRange& subresourceRange, VkImageLayout layout, u32 rowPitch, u32 sliceHeight);

			/**
			 * Copies the contents of the image subresource into the destination buffer. Caller must ensure the provided
			 * extents are within valid bounds of the image and that the provided buffer is large enough.
			 *
			 * @param	source				Source image to copy from.
			 * @param	destination			Destination buffer to copy to.
			 * @param	region				Region of the image to copy from.
			 * @param	subresourceRange	Subresource(s) of the image to copy from.
			 * @param	layout				Current layout of the image subresources in the provided range.
			 * @param	rowPitch			Determines how many pixels to advance when moving to a new row in the destination buffer.
			 * @param	sliceHeight			Determines how many pixels to advance when moving to a new slice in the destination buffer.
			 */
			void CopyImageToBuffer(VulkanImage* source, VulkanBuffer* destination, const VkExtent3D& region, const VkImageSubresourceRange& subresourceRange, VkImageLayout layout, u32 rowPitch, u32 sliceHeight);

			/**
			 * Copies one or multiple regions from one or multiple image sub-resources from the source image to the destination image.
			 * Caller must ensure the region extents and sub-resources are valid for both source and destination images.
			 *
			 * @param	source						Source image to copy from.
			 * @param	destination					Destination image to copy to.
			 * @param	sourceLayout				Current layout of the source image subresources in the provided range.
			 * @param	destinationLayout			Current layout of the destination image subresources in the provided range.
			 * @param	sourceSubresourceRange		Subresource(s) of the image to copy from.
			 * @param	destinationSubresourceRange	Subresource(s) of the image to copy to.
			 * @param	regionCount					Number of regions in the @p regions array.
			 * @param	regions						One or multiple regions which to copy.
			 */
			void CopyImageToImage(VulkanImage* source, VulkanImage* destination, VkImageLayout sourceLayout, VkImageLayout destinationLayout, const VkImageSubresourceRange& sourceSubresourceRange, const VkImageSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageCopy* regions);

			/**
			 * Blits one or multiple regions from one or multiple image sub-resources from the source image to the destination image.
			 * Caller must ensure the region extents and sub-resources are valid for both source and destination images.
			 *
			 * @param	source						Source image to blit from.
			 * @param	destination					Destination image to blit to.
			 * @param	sourceLayout				Current layout of the source image subresources in the provided range.
			 * @param	destinationLayout			Current layout of the destination image subresources in the provided range.
			 * @param	sourceSubresourceRange		Subresource(s) of the image to blit from.
			 * @param	destinationSubresourceRange	Subresource(s) of the image to blit to.
			 * @param	regionCount					Number of regions in the @p regions array.
			 * @param	regions						One or multiple regions which to blit.
			 */
			void Blit(VulkanImage* source, VulkanImage* destination, VkImageLayout sourceLayout, VkImageLayout destinationLayout, const VkImageSubresourceRange& sourceSubresourceRange, const VkImageSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageBlit* regions);

			/**
			 * Resolves multisampled images into non-multiplesampled ones, from one or multiple regions from one or multiple image sub-resources
			 * from the source image to the destination image. Caller must ensure the region extents and sub-resources are valid for both source
			 * and destination images. Source image must have multiple samples while the destination image must have a single sample. Samples from
			 * the source image will be resolved into a single sample in the destination image.
			 * 
			 * @param	source						Source image to resolve.
			 * @param	destination					Destination image to write the resolved data into.
			 * @param	sourceLayout				Current layout of the source image subresources in the provided range.
			 * @param	destinationLayout			Current layout of the destination image subresources in the provided range.
			 * @param	sourceSubresourceRange		Subresource(s) of the image to resolve.
			 * @param	destinationSubresourceRange	Subresource(s) of the image to resolve to.
			 * @param	regionCount					Number of regions in the @p regions array.
			 * @param	regions						One or multiple regions which to resolve.
			 */
			void Resolve(VulkanImage* source, VulkanImage* destination, VkImageLayout sourceLayout, VkImageLayout destinationLayout, const VkImageSubresourceRange& sourceSubresourceRange, const VkImageSubresourceRange& destinationSubresourceRange, uint32_t regionCount, VkImageResolve* regions);

			/**
			 * Returns the current layout of the specified image, as seen by this command buffer. This is different from the
			 * global layout stored in VulkanImage itself, as it includes any transitions performed by the command buffer
			 * (at the current point in time), while the global layout is only updated after a command buffer as been submitted.
			 *
			 * @param[in]	image			Image to lookup the layout for.
			 * @param[in]	range			Subresource range of the image to lookup the layout for.
			 * @param[in]	inRenderPass	If true this will return the layout of the image after the render pass begins.
			 *								If false it will return the current layout of the image. These may be different
			 *								in the case the image is used in the framebuffer, in which case the render pass
			 *								may perform an automated layout transition when it begins.
			 */
			VkImageLayout GetCurrentLayout(VulkanImage* image, const VkImageSubresourceRange& range, bool inRenderPass);

		private:
			friend class VulkanCmdBufferPool;
			friend class VulkanCommandBuffer;
			friend class VulkanQueue;
			friend class VulkanHardwareBuffer;
			friend class VulkanTexture;

			/** Contains information about a single Vulkan resource bound/used on this command buffer. */
			struct ResourceUseHandle
			{
				bool Used;
				VulkanAccessFlags Flags;
			};

			/** Describes where and how is a resource being accessed and by which stages. */
			struct ResourcePipelineUse
			{
				/** Specifies how will the subresource be accessed during the current render pass or dispatch call. */
				VulkanAccessFlags Access;

				/** Stages the image is being used in during the current render pass or dispatch call. */
				VkPipelineStageFlags Stages = 0;
			};

			/** Contains information about a single Vulkan buffer resource bound/used on this command buffer. */
			struct BufferInfo
			{
				ResourceUseHandle UseHandle;

				BufferUseFlags UseFlags;

				/**
				 * Use flags when buffer is bound for any kind of operation that will require an execution or memory
				 * barrier due to a write hazard. Currently used for issuing execution/memory barriers after shader writes
				 * (not counting transfer operations which handle the barriers explicitly). Reset after a memory barrier is
				 * issued.
				 */
				ResourcePipelineUse WriteHazardUse;
			};

			/** Contains information about a single Vulkan image resource bound/used on this command buffer. */
			struct ImageInfo
			{
				ResourceUseHandle UseHandle;

				u32 SubresourceInfoIdx;
				u32 NumSubresourceInfos;
			};

			/** Contains information about a range of Vulkan image sub-resources bound/used on this command buffer. */
			struct ImageSubresourceInfo
			{
				VkImageSubresourceRange Range;

				// Storing stage & access flags separately per use category so they can be cleared independantly when that use
				// ends (e.g. image unbound as FB attachment, or memory barrier executed)

				/** Use flags when subresource is bound for shader reads or writes. Reset after resource is unbound. */
				ResourcePipelineUse ShaderUse;

				/** Use flags when subresource is bound as a framebuffer attachment. Reset after resource is unbound. */
				ResourcePipelineUse FbUse;

				/** Use flags when subresource is bound for a transfer operation. Currently unused. */
				ResourcePipelineUse TransferUse;

				/**
				 * Use flags when subresource is bound for any kind of operation that will require an execution or memory
				 * barrier due to a write hazard. Currently used for issuing execution/memory barriers after shader writes
				 * (not counting render pass writes, which handles barriers through subpass dependencies, or transfer operations
				 * which handle the barriers explicitly). Reset after a memory barrier is issued.
				 */
				ResourcePipelineUse WriteHazardUse;

				/**
				 * Specifies how will the subresource be used during the current render pass or dispatch call. Reset
				 * after use.
				 */
				ImageUseFlags UseFlags;

				/** Determines is the initial use of this subresource read-only. Used for better determining access flags. */
				bool InitialReadOnly = false;

				// Only relevant for layout transitions
				/**
				 * Layout transition performed during the submit() call. Doesn't require ending the render pass since it
				 * will be delayed until submit().
				 */
				VkImageLayout InitialLayout;

				/**
				 * Layout the image is currently in. This will be the initial layout if no other transition was performed, or
				 * layout resulting from the last performed transition.
				 */
				VkImageLayout CurrentLayout;

				/**
				 * Stores the layout that the image needs to be before being used in the current render pass or dispatch call.
				 * Equal to currentLayout if no transition is needed. Updated after every render pass or dispatch call.
				 */
				VkImageLayout RequiredLayout;

				/**
				 * Layout the image will have after the render pass executes, taking account automatic transitions render pass
				 * does on its attachments. Only relevant for FB attachments. Ignored if render pass doesn't execute.
				 */
				VkImageLayout RenderPassLayout;
			};

			/**
			 * Ends render pass recording (as started with BeginRenderPass().
			 *
			 * @param isInternalInterrupt		This will be true if we're ending the render pass temporarily, as a requirement for some internal operation (such as memory barriers), rather than explicitly requested by the user.
			 */
			void EndRenderPass(bool isInternalInterrupt);

			/** Checks if all the prerequisites for rendering have been made (e.g. render target and pipeline state are set.) */
			bool IsReadyForRender();

			/** Marks the command buffer as submitted on a queue. */
			void SetIsSubmitted() { mState = State::Submitted; }

			/** Binds the current graphics pipeline to the command buffer. Returns true if bind was successful. */
			bool BindGraphicsPipeline();

			/**
			 * Binds any dynamic states to the pipeline, as required.
			 *
			 * @param[in]	forceAll	If true all states will be bound. If false only states marked as dirty will be bound.
			 */
			void BindDynamicStates(bool forceAll);

			/** Binds vertex and index buffers to the pipeline, if dirty. */
			void BindVertexInputs();

			/** Binds the currently stored GPU parameters object, if dirty. */
			void BindGpuParams();

			/**
			 * Clears the specified area of the currently bound render target. If in the middle of the render pass this will issue a clear command,
			 * but if render pass has not begun yet it will instead attempt to perform the clear at the next render pass start.
			 */
			void ClearViewport(const Rect2I& area, u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask);

			/**
			 * Executes a clear command in the command buffer.
			 *
			 * @param area			Area in the currently bound render target to clear.
			 * @param clearMask		Mask specifying which surfaces of the currently bound render target to clear.
			 * @param clearValues	Values used for clearing attachments.
			 */
			void ExecuteClearCommand(const Rect2I& area, RenderSurfaceMask clearMask, const Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1>& clearValues);

			/** Starts and ends a render pass, intended only for a clear operation. */
			void ExecuteClearPass();

			/** Executes any queued layout transitions by issuing a pipeline barrier. */
			void ExecuteLayoutTransitions();

			/** Executes any queued memory barriers. */
			void ExecuteWriteHazardBarrier();

			/**
			 * Updates final layouts for images used by the current framebuffer, reflecting layout changes performed by render
			 * pass' automatic layout transitions.
			 */
			void UpdateFinalLayouts();

			/**
			 * Lets the command buffer know that the provided image subresource will be used in subsequent draw or dispatch
			 * calls. Transitions the image to @p layout (if needed).
			 *
			 * @param[in]	image					Image to register with the command buffer.
			 * @param[in]	range					Sub-resource range of the image that affected.
			 * @param[in]	use						Intended use for the resource.
			 * @param[in]	layout					Layout the image needs to be transitioned in before use. Set to undefined
			 *										layout if no transition is required.
			 * @param[in]	finalLayout				Final layout transitioned into by a render pass. Only relevant if image
			 *										is being used as a framebuffer attachment.
			 * @param[in]	access					Flags that determine will the resource be written or read from (or both).
			 * @param[in]	stages					Set of stages that read/write from/to the resource.
			 */
			void RegisterResource(VulkanImage* image, const VkImageSubresourceRange& range, ImageUseFlagBits use, VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Updates an existing image sub-resource with new layout, access and stage flags for the purposes of shader
			 * read or write. Sets up any necessary execution and memory barriers, as well as layout transitions.
			 */
			void UpdateShaderSubresource(VulkanImage* image, u32 imageInfoIdx, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Updates an existing image sub-resource with new layout, access and stage flags for the purposes of being bound
			 * as a framebuffer attachment. Sets up any necessary execution and memory barriers, as well as layout transitions.
			 */
			void UpdateFramebufferSubresource(VulkanImage* image, u32 imageInfoIdx, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VkImageLayout finalLayout, VulkanAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Updates an existing image sub-resource with new access and stage flags for the purposes of being used for a
			 * transfer operation. Sets up any necessary execution and memory barriers, as well as layout transitions.
			 */
			void UpdateTransferSubresource(VulkanImage* image, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VulkanAccessFlags access, VkPipelineStageFlags stages);

			/** Finds a subresource info structure containing the specified face and mip level of the provided image. */
			ImageSubresourceInfo& FindSubresourceInfo(VulkanImage* image, u32 face, u32 mip);

			/** Gets all queries registered on this command buffer that haven't been ended. */
			void GetInProgressQueries(Vector<VulkanTimerQuery*>& timer, Vector<VulkanOcclusionQuery*>& occlusion) const;

			/** Returns the read mask for the current framebuffer. */
			RenderSurfaceMask GetFramebufferReadMask();

			/** Returns the current viewport area in pixels. This depends on the currently bound framebuffer and normalized viewport area. */
			Rect2I GetViewportArea() const;

			/** Returns the current area of the render pass in pixels. This depends on the currently bound framebuffer. */
			Rect2I GetRenderPassArea() const;

			/** Notifies the active render target that a rendering command was queued that will potentially change its contents. */
			void NotifyRenderTargetModified();

			u32 mId;
			u32 mQueueFamily;
			State mState = State::Ready;
			VulkanDevice& mDevice;
			VulkanCommandBuffer* mOwner = nullptr;
			VkCommandPool mPool;
			VkCommandBuffer mCmdBuffer;
			VkFence mFence;

			VulkanSemaphore* mIntraQueueSemaphore = nullptr;
			VulkanSemaphore* mInterQueueSemaphores[BS_MAX_VULKAN_CB_DEPENDENCIES]{};
			mutable u32 mNumUsedInterQueueSemaphores = 0;

			VulkanFramebuffer* mFramebuffer = nullptr;
			u32 mRenderTargetReadOnlyFlags = 0;
			RenderSurfaceMask mRenderTargetLoadMask = RT_NONE;

			UnorderedMap<VulkanResource*, ResourceUseHandle> mResources;
			UnorderedMap<VulkanResource*, u32> mImages;
			UnorderedMap<VulkanResource*, BufferInfo> mBuffers;
			UnorderedMap<VulkanSwapChain*, ResourceUseHandle> mSwapChains;
			UnorderedSet<VulkanOcclusionQuery*> mOcclusionQueries;
			UnorderedSet<VulkanTimerQuery*> mTimerQueries;
			Vector<ImageInfo> mImageInfos;
			Vector<ImageSubresourceInfo> mSubresourceInfoStorage;
			Set<u32> mShaderBoundSubresourceInfos;
			u32 mGlobalQueueIdx = -1;

			bool mNeedsWARMemoryBarrier : 1;
			bool mNeedsRAWMemoryBarrier : 1;
			VkPipelineStageFlags mMemoryBarrierSrcStages = 0;
			VkPipelineStageFlags mMemoryBarrierDstStages = 0;
			VkAccessFlags mMemoryBarrierSrcAccess = 0;
			VkAccessFlags mMemoryBarrierDstAccess = 0;

			SPtr<VulkanGraphicsPipelineState> mGraphicsPipeline;
			SPtr<VulkanComputePipelineState> mComputePipeline;
			SPtr<VertexDeclaration> mVertexDecl;
			SPtr<VulkanIndexBuffer> mIndexBuffer;
			Vector<SPtr<VulkanVertexBuffer>> mVertexBuffers;
			Rect2 mNormalizedViewportArea{ 0.0f, 0.0f, 1.0f, 1.0f };
			Rect2I mScissor{ 0, 0, 0, 0 };
			u32 mStencilRef = 0;
			DrawOperationType mDrawOp = DOT_TRIANGLE_LIST;
			u32 mRequiredVertexBufferBindingCount = 0;
			u32 mBoundDescriptorSetCount = 0;
			bool mGfxPipelineRequiresBind : 1;
			bool mCmpPipelineRequiresBind : 1;
			bool mViewportRequiresBind : 1;
			bool mStencilRefRequiresBind : 1;
			bool mScissorRequiresBind : 1;
			bool mBoundParamsDirty : 1;
			bool mVertexInputsDirty : 1;
			bool mIsRenderPassInterrupted = false;
			DescriptorSetBindFlags mDescriptorSetsBindState;
			SPtr<VulkanGpuParams> mBoundParams;

			std::array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> mClearValues{};
			RenderSurfaceMask mClearMask;
			Rect2I mClearArea;

			Vector<VulkanSemaphore*> mSemaphoresTemp{ BS_MAX_UNIQUE_QUEUES };
			VkBuffer mVertexBuffersTemp[BS_MAX_BOUND_VERTEX_BUFFERS]{};
			VkDeviceSize mVertexBufferOffsetsTemp[BS_MAX_BOUND_VERTEX_BUFFERS]{};
			VkDescriptorSet* mDescriptorSetsTemp;
			UnorderedMap<u32, TransitionInfo> mTransitionInfoTemp;
			Vector<VkImageMemoryBarrier> mLayoutTransitionBarriersTemp;
			UnorderedMap<VulkanImage*, u32> mQueuedLayoutTransitions;
			Vector<VulkanEvent*> mQueuedEvents;
			Vector<VulkanQuery*> mQueuedQueryResets;
			UnorderedSet<VulkanSwapChain*> mActiveSwapChains;

			SPtr<RenderTarget> mRenderTarget;
			bool mRenderTargetModified = false;
		};

		/** CommandBuffer implementation for Vulkan. */
		class VulkanCommandBuffer : public CommandBuffer
		{
		public:
			~VulkanCommandBuffer();
		
			/**
			 * Submits the command buffer for execution.
			 *
			 * @param[in]	syncMask	Mask that controls which other command buffers does this command buffer depend upon
			 *							(if any). See description of @p syncMask parameter in RenderAPI::executeCommands().
			 */
			void Submit(u32 syncMask);

			/** Called by the backend when we have been notified the command buffer has finished executing on the GPU. */
			void NotifyExecutionCompleted();

			/**
			 * Returns the internal command buffer.
			 *
			 * @note	This buffer will change after a submit() call.
			 */
			VulkanCmdBuffer* GetInternal() const { return mBuffer; }

			CommandBufferState GetState() const override;
			void Reset() override;

		private:
			friend class VulkanCommandBufferManager;

			VulkanCommandBuffer(VulkanDevice& device, GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary);

			/**
			 * Tasks the command buffer to find a new internal command buffer. Call this after the command buffer has been
			 * submitted to a queue (it's not allowed to be used until the queue is done with it).
			 */
			void AcquireNewBuffer();

			VulkanCmdBuffer* mBuffer;
			VulkanDevice& mDevice;
			VulkanQueue* mQueue;
			u32 mIdMask;
		};

		/** @} */
	} // namespace ct
} // namespace bs
