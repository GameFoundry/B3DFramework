//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "B3DVulkanResource.h"
#include "B3DVulkanGpuPipelineState.h"
#include "B3DVulkanGpuDevice.h"
#include "Math/B3DArea2.h"
#include "Math/B3DArea2.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"
#include "Utility/B3DDenseMap.h"

namespace b3d
{
	namespace render
	{
		class VulkanOcclusionQuery;
		class VulkanTimerQuery;
		class VulkanImage;

		/** @addtogroup Vulkan
		 *  @{
		 */

// Maximum number of command buffers that another command buffer can be dependant on (via a sync mask)
#define BS_MAX_VULKAN_CB_DEPENDENCIES 2

		/** Wrapper around a Vulkan semaphore object that manages its usage and lifetime. */
		class VulkanSemaphore : public VulkanResource
		{
		public:
			VulkanSemaphore(VulkanResourceManager* owner, const StringView& name = "");
			~VulkanSemaphore();

			/** Returns the internal handle to the Vulkan object. */
			VkSemaphore GetHandle() const { return mSemaphore; }

		private:
			VkSemaphore mSemaphore;
		};

		class VulkanInternalCommandBuffer;

		/** Vulkan implementation of GpuCommandBufferPool. */
		class VulkanGpuCommandBufferPool : public GpuCommandBufferPool
		{
			using Base = GpuCommandBufferPool;
		public:
			VulkanGpuCommandBufferPool(VulkanGpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation);
			~VulkanGpuCommandBufferPool() override;

			SPtr<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) override;
			SPtr<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) override;
			void Reset() override;
			void Destroy() override;

		private:
			VkCommandPool mVulkanPool = VK_NULL_HANDLE;
			u32 mQueueFamily = ~0u;
			u32 mNextCommandBufferId = 1;

			UnorderedMap<u32, SPtr<VulkanGpuCommandBuffer>> mCommandBuffers;
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

		/** All the information required for submitting a VulkanGpuCommandBuffer */
		struct GpuCommandBufferSubmitInformation
		{
			SPtr<VulkanGpuCommandBuffer> QueryResetCommandBuffer; /**< Helper command buffer on which to reset queries. Should be submitted first. */
			SPtr<VulkanGpuCommandBuffer> SourceQueueTransitionCommandBuffer[GQT_COUNT]; /**< Contains resource transitions from their current queue to the destination queue, if there is a queue change. May be empty if there are no queue changes. To be executed on the source queue, rather than on the queue you are submitting on. */
			SPtr<VulkanGpuCommandBuffer> DestinationQueueTransitionCommandBuffer; /**< Contains image layout transitions and transitions from source to the destination queue, if there are any. Should be submitted after the query reset command buffer. This submit should contain the provided semaphores if not empty. */
			SPtr<VulkanGpuCommandBuffer> PrimaryCommandBuffer; /**< Primary command buffer we're submitting. This should be submitted after the destination queue transition command buffer. This submit should contain the semaphores if destination queue transition command buffer is not present. */
			TInlineArray<VulkanSemaphore*, 8> Semaphores; /**< Semaphores that need to be waited on before executing the command buffers. */
		};

		/** CommandBuffer implementation for Vulkan. */
		class VulkanGpuCommandBuffer final : public GpuCommandBuffer
		{
		private:
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
			~VulkanGpuCommandBuffer() override;

			void SetName(const StringView& name) override;
			CommandBufferState GetState() const override;

			void SetGpuParameters(const SPtr<GpuParameters>& parameters) override;
			void SetDynamicBufferOffset(u32 bufferIndex, u32 offset) override;
			void SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState) override;
			void SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState) override;
			void SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount) override;
			void SetIndexBuffer(const SPtr<GpuBuffer>& buffer) override;
			void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription) override;
			void SetDrawOperation(DrawOperationType operation) override;
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override;
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override;
			void DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ) override;
			void BeginRenderPass(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask) override;
			void EndRenderPass() override { EndRenderPass(false); }
			bool IsInRenderPass() const override { return mState == State::RecordingRenderPass; }
			void SetViewport(const Area2& area) override;
			void ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask) override;
			void ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask) override;
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) override;
			void DisableScissorTest() override;
			void SetStencilReferenceValue(u32 value) override;
			void CopyBufferToBuffer(const SPtr<GpuBuffer>& source, const SPtr<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length) override;
			void WriteTimestamp(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool) override;
			void BeginQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool, GpuQueryFlags flags) override;
			void EndQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool) override;
			void ResetQueries(const SPtr<GpuQueryPool>& queryPool) override;
			void BeginLabel(const StringView& name) override;
			void EndLabel() override;
			void InsertLabel(const StringView& name) override;
			void End() override;
			void TransitionTextureLayout(const SPtr<Texture>& texture, GpuTextureLayout layout, const GpuTextureSubresourceRange& subresourceRange) override;

			/** Returns an unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

			/** Returns the thread that the command buffer is allowed to be used on. */
			ThreadId GetOwnerThread() const { return mOwnerThread; }

			/**
			 * Prepares the command buffer to be submitted on a GpuQueue.
			 *
			 * @param queueUsage			Usage of the queue the command buffer will be submitted on.
			 * @param queueIndex			Index of the queue the command buffer will be submitted on.
			 * @return						Information required for submitting the command buffer on the queue.
			 * 
			 * @note Submit thread only.
			 */
			GpuCommandBufferSubmitInformation PrepareForSubmitOnSubmitThread(GpuQueueUsage queueUsage, u32 queueIndex);

			/** Called when the command buffer is about to be sent to the submit queue for submit. */
			void NotifyWillQueueForSubmit();

			/** Returns the handle to the internal Vulkan command buffer wrapped by this object. */
			VkCommandBuffer GetVulkanHandle() const { return mCommandBufferHandle; }

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
			 * Allocates a new set of semaphores that will be signaled when the command buffer finishes execution.
			 * Releases the previously allocated semaphores, if they exist. Use GetIntraQueueSemaphore() &
			 * RequestInterQueueSemaphore() to retrieve latest allocated semaphores.
			 *
			 * @param	outSemaphores	Output array to append all allocated semaphores in. 
			 */
			u32 AllocateSignalSemaphores(TInlineArray<VkSemaphore, 8>& outSemaphores);

			/** Returns true if the command buffer is currently being processed by the device. */
			bool IsSubmitted() const { return mState == State::Submitted; }

			/** Returns true if the command buffer is currently recording (but not within a render pass). */
			bool IsRecording() const { return mState == State::Recording || mState == State::RecordingRenderPass; }

			/** Returns true if the command buffer is ready to be submitted to a queue. */
			bool IsReadyForSubmit() const { return mState == State::RecordingDone; }

			/** Returns true if the command buffer is done executing on the device. */
			bool IsDone() const { return mState == State::Done; }

			/**
			 * Checks is the command buffer still executing on the GPU. Internal state will be updated if execution finishes.
			 *
			 * @param	block	If true, the system will block until the command buffer is done executing.
			 * @return			True if execution has finished (or was never submitted), false if still running.
			 *
			 * @note	Submit thread only.
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
			void RegisterResource(VulkanResource* res, GpuAccessFlags flags);

			/**
			 * Lets the command buffer know that the provided image will be used for shader reads or writes in a subsequent draw
			 * or dispatch call. Transfers the image to the provided layout and issues any necessary execution and memory
			 * barriers.
			 */
			void RegisterImageShader(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, GpuAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Lets the command buffer know that the provided image will be used as a framebuffer attachment in a subsequent
			 * draw call. Transfers the image to the provided layout and issues any necessary execution and memory barriers.
			 */
			void RegisterImageFramebuffer(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, VkImageLayout finalLayout, GpuAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Lets the command buffer know that the provided image will be used for a transfer operation. Transfers the image
			 * to the provided layout and issues any necessary execution and memory barriers.
			 */
			void RegisterImageTransfer(VulkanImage* image, const VkImageSubresourceRange& range, VkImageLayout layout, GpuAccessFlags access);
			/**
			 * Lets the command buffer know that the provided image resource has been queued on it, and will be used by the
			 * device when the command buffer is submitted. @p stages can be left empty for all uses except for generic and
			 * parameter buffer types.
			 */
			void RegisterBuffer(VulkanBuffer* res, BufferUseFlagBits useFlags, GpuAccessFlags access, VkPipelineStageFlags stages = 0);

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

			/************************************************************************/
			/* 								COMMANDS	                     		*/
			/************************************************************************/

			/**
			 * Registers a command that signals the event when executed. Will be delayed until the end of the current
			 * render pass, if any.
			 */
			void SetEvent(VulkanEvent* event);

			/**
			 * Issues a pipeline barrier on the provided buffer. See vkCmdPipelineBarrier in Vulkan spec. for usage
			 * information.
			 */
			void MemoryBarrier(VkBuffer buffer, VkAccessFlags sourceAccessFlags, VkAccessFlags destinationAccessFlags, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage);

			/** Issues a pipeline barrier on the provided buffer. Uses the default pipeline stages for provided access flags.  */
			void MemoryBarrier(VkBuffer buffer, VkAccessFlags sourceAccessFlags, VkAccessFlags destinationAccessFlags);

			/**
			 * Copies the provided memory into the buffer.
			 *
			 * @param	destination		Destination to copy into.
			 * @param	data			Data to copy into.
			 * @param	offset			Offset in the destination buffer to copy to, in bytes. Must be a multiple of 4.
			 * @param	length			Size of the data to copy, in bytes. Must be a multiple of 4 and less or equal than 65536.
			 */
			void UpdateBuffer(VulkanBuffer* destination, u8* data, VkDeviceSize offset, VkDeviceSize length);

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
			 */
			void CopyBufferToImage(VulkanBuffer* source, VulkanImage* destination, const VkExtent3D& region, const VkImageSubresourceRange& subresourceRange, VkImageLayout layout);

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

			/** Returns a set of pipeline stages that can are allowed to be used for the specified set of access flags. */
			static VkPipelineStageFlags GetPipelineStageFlags(VkAccessFlags accessFlags);

		private:
			friend class VulkanGpuCommandBufferPool;
			friend class VulkanGpuCommandBuffer;
			friend class VulkanGpuQueue;
			friend class VulkanGpuBuffer;
			friend class VulkanTexture;

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

				/**
				 * Use flags to set after running the pipeline barrier. Ensures that resource accesses after the barrier
				 * trigger barriers on their next use.
				 */
				ResourcePipelineUse NewWriteHazardUse;
			};

			/** Contains information about a single Vulkan image resource bound/used on this command buffer. */
			struct ImageInfo
			{
				ResourceUseHandle UseHandle;

				u32 FirstSubresourceInfoIndex;
				u32 SubresourceInfoCount;
			};

			/** Information an acquires swap chain image. */
			struct SwapChainImageInformation
			{
				VulkanSwapChain* SwapChain = nullptr;
				u32 ImageIndex = ~0u;
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
				ResourcePipelineUse FramebufferUse;

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
				 * Use flags to set after running the pipeline barrier. Ensures that resource accesses after the barrier
				 * trigger barriers on their next use.
				 */
				ResourcePipelineUse NewWriteHazardUse;

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
			};

			/** Information about queries recorded on the command buffer. */
			struct QueryInformation
			{
				QueryInformation(bool isInRenderPass = false, GpuQueryType type = GpuQueryType::Timestamp, u64 poolIdentifier = 0)
					: IsInRenderPass(isInRenderPass), Type(type), PoolIdentifier(poolIdentifier)
				{ }

				bool IsInRenderPass = false;
				GpuQueryType Type = GpuQueryType::Timestamp;
				u64 PoolIdentifier = 0;
			};

			VulkanGpuCommandBuffer(VulkanGpuDevice& device, VulkanGpuCommandBufferPool& pool, u32 id, VkCommandBuffer commandBufferHandle, ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation);

			/** Returns the pool the command buffer was allocated from. */
			VulkanGpuCommandBufferPool& GetPool() const { return mPool; }

			/** Makes the command buffer ready to start recording commands. */
			void Begin();

			/** Begins render pass recording. Must be called within begin()/end() calls. */
			void BeginRenderPass();

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
			void ClearViewport(const Area2I& area, u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask);

			/**
			 * Executes a clear command in the command buffer.
			 *
			 * @param area			Area in the currently bound render target to clear.
			 * @param clearMask		Mask specifying which surfaces of the currently bound render target to clear.
			 * @param clearValues	Values used for clearing attachments.
			 */
			void ExecuteClearCommand(const Area2I& area, RenderSurfaceMask clearMask, const Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1>& clearValues);

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
			void RegisterResource(VulkanImage* image, const VkImageSubresourceRange& range, ImageUseFlagBits use, VkImageLayout layout, VkImageLayout finalLayout, GpuAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Updates an existing image sub-resource with new layout, access and stage flags for the purposes of shader
			 * read or write. Sets up any necessary execution and memory barriers, as well as layout transitions.
			 */
			void UpdateShaderSubresource(VulkanImage* image, u32 imageInfoIdx, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, GpuAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Updates an existing image sub-resource with new layout, access and stage flags for the purposes of being bound
			 * as a framebuffer attachment. Sets up any necessary execution and memory barriers, as well as layout transitions.
			 */
			void UpdateFramebufferSubresource(VulkanImage* image, u32 imageInfoIdx, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, VkImageLayout finalLayout, GpuAccessFlags access, VkPipelineStageFlags stages);

			/**
			 * Updates an existing image sub-resource with new access and stage flags for the purposes of being used for a
			 * transfer operation. Sets up any necessary execution and memory barriers, as well as layout transitions.
			 */
			void UpdateTransferSubresource(VulkanImage* image, ImageSubresourceInfo& subresourceInfo, VkImageLayout layout, GpuAccessFlags access, VkPipelineStageFlags stages);

			/** Finds a subresource info structure containing the specified face and mip level of the provided image. */
			ImageSubresourceInfo& FindSubresourceInfo(VulkanImage* image, u32 face, u32 mip);

			/** Returns the read mask for the current framebuffer. */
			RenderSurfaceMask GetFramebufferReadMask();

			/** Returns the current viewport area in pixels. This depends on the currently bound framebuffer and normalized viewport area. */
			Area2I GetViewportArea() const;

			/** Returns the current area of the render pass in pixels. This depends on the currently bound framebuffer. */
			Area2I GetRenderPassArea() const;

			/** Notifies the active render target that a rendering command was queued that will potentially change its contents. */
			void NotifyRenderTargetModified();

			/** Returns the owner GPU device, cast as a VulkanGpuDevice. */
			VulkanGpuDevice& GetVulkanGpuDevice() const { return static_cast<VulkanGpuDevice&>(mGpuDevice); }

			u32 mId;
			State mState = State::Ready;
			VkCommandBuffer mCommandBufferHandle;
			VulkanGpuCommandBufferPool& mPool;
			VkFence mFence;
			ThreadId mOwnerThread;

			VulkanSemaphore* mIntraQueueSemaphore = nullptr;
			VulkanSemaphore* mInterQueueSemaphores[BS_MAX_VULKAN_CB_DEPENDENCIES]{};
			mutable u32 mNumUsedInterQueueSemaphores = 0;

			VulkanFramebuffer* mFramebuffer = nullptr;
			u32 mRenderTargetReadOnlyFlags = 0;
			RenderSurfaceMask mRenderTargetLoadMask = RT_NONE;

			DenseMap<VulkanResource*, ResourceUseHandle> mResources;
			DenseMap<VulkanResource*, u32> mImages;
			DenseMap<VulkanResource*, BufferInfo> mBuffers;
			UnorderedMap<VulkanSwapChain*, ResourceUseHandle> mSwapChains;
			Vector<ImageInfo> mImageInfos;
			Vector<ImageSubresourceInfo> mSubresourceInfoStorage;
			Set<u32> mShaderBoundSubresourceInfos;
			GpuQueueId mSubmittedQueueId;

			bool mNeedsWARMemoryBarrier : 1;
			bool mNeedsRAWMemoryBarrier : 1;
			VkPipelineStageFlags mMemoryBarrierSrcStages = 0;
			VkPipelineStageFlags mMemoryBarrierDstStages = 0;
			VkAccessFlags mMemoryBarrierSrcAccess = 0;
			VkAccessFlags mMemoryBarrierDstAccess = 0;

			SPtr<VulkanGpuGraphicsPipelineState> mGraphicsPipeline;
			SPtr<VulkanGpuComputePipelineState> mComputePipeline;
			SPtr<VertexDescription> mVertexDescription;
			SPtr<VulkanGpuBuffer> mIndexBuffer;
			Vector<SPtr<VulkanGpuBuffer>> mVertexBuffers;
			Area2 mNormalizedViewportArea{ 0.0f, 0.0f, 1.0f, 1.0f };
			Area2I mScissor{ 0, 0, 0, 0 };
			bool mIsScissorTestEnabled = false;
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
			bool mIsDebugLabelOpen = false;
			DescriptorSetBindFlags mDescriptorSetsBindState;
			SPtr<VulkanGpuParameters> mBoundParams;

			std::array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> mClearValues{};
			RenderSurfaceMask mClearMask;
			Area2I mClearArea;

			VkBuffer mVertexBuffersTemp[BS_MAX_BOUND_VERTEX_BUFFERS]{};
			VkDeviceSize mVertexBufferOffsetsTemp[BS_MAX_BOUND_VERTEX_BUFFERS]{};
			VkDescriptorSet* mDescriptorSetsTemp;
			TransitionInfo mTransitionInfoTemp[GQT_COUNT];
			Vector<VkImageMemoryBarrier> mLayoutTransitionBarriersTemp;
			UnorderedMap<VulkanImage*, u32> mQueuedLayoutTransitions;
			Vector<VulkanEvent*> mQueuedEvents;
			Vector<SwapChainImageInformation> mAcquiredSwapChainImages;
			Vector<u32> mDynamicDescriptorOffsetsToBind;
			UnorderedMap<u32, u32> mDynamicDescriptorOffsetsOverrides;

			SPtr<RenderTarget> mRenderTarget;
			bool mRenderTargetModified = false;

#if B3D_BUILD_TYPE == B3D_BUILD_TYPE_DEVELOPMENT
			Vector<QueryInformation> mOpenQueries; // Only used for validation
#endif
		};

		/** @} */
	} // namespace render
} // namespace b3d
