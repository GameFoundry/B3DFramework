//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "B3DVulkanResource.h"
#include "B3DVulkanGpuPipelineState.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanResourceTracker.h"
#include "B3DVulkanUtility.h"
#include "B3DIVulkanRenderWindowSurface.h"
#include "Allocators/B3DPoolAlloc.h"
#include "Math/B3DArea2.h"
#include "Math/B3DArea2.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"
#include "Utility/B3DDenseMap.h"
#include "Utility/B3DVulkanBarrierHelper.h"

namespace b3d
{
	namespace render
	{
		class VulkanOcclusionQuery;
		class VulkanTimerQuery;
		class VulkanImage;
		class VulkanBarrierHelper;

		/** @addtogroup Vulkan
		 *  @{
		 */

// Maximum number of command buffers that another command buffer can be dependant on (via a sync mask)
#define B3D_MAX_COMMAND_BUFFER_DEPENDENCIES 2

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
		public:
			~VulkanGpuCommandBuffer() override;

			void SetName(const StringView& name) override;
			void SetGpuParameterSet(const SPtr<GpuParameterSet>& parameterSet) override;
			void SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset) override;
			void SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState) override;
			void SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState) override;
			void SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount) override;
			void SetIndexBuffer(const SPtr<GpuBuffer>& buffer) override;
			void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription) override;
			void SetDrawOperation(DrawOperationType operation) override;
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override;
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, u32 firstInstance) override;
			void DispatchCompute(u32 groupCountX, u32 groupCountY, u32 groupCountZ) override;
			void BeginRenderPass(const RenderPassCreateInformation& createInformation) override;
			void EndRenderPass() override;
			bool IsInRenderPass() const override { return mState == GpuCommandBufferState::RecordingRenderPass; }
			void SetViewport(const Area2& area) override;
			void ClearRenderTarget(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil) override;
			void ClearViewport(RenderSurfaceMask mask, const Color& color, float depth, u16 stencil) override;
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) override;
			void DisableScissorTest() override;
			void SetStencilReferenceValue(u32 value) override;
			void CopyBufferToBuffer(const SPtr<GpuBuffer>& source, const SPtr<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length) override;
			void CopyBufferToTexture(const SPtr<GpuBuffer>& source, const SPtr<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer) override;
			void CopyTextureToBuffer(const SPtr<Texture>& source, const SPtr<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset) override;
			bool CopyTexture(const SPtr<Texture>& source, const SPtr<Texture>& destination, const TextureCopyInformation& copyInformation) override;
			bool BlitTexture(const SPtr<Texture>& source, const SPtr<Texture>& destination, const TextureBlitInformation& blitInformation) override;
			void WriteTimestamp(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool) override;
			void BeginQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool, GpuQueryFlags flags) override;
			void EndQuery(GpuQueryId query, const SPtr<GpuQueryPool>& queryPool) override;
			void ResetQueries(const SPtr<GpuQueryPool>& queryPool) override;
			void BeginLabel(const StringView& name) override;
			void EndLabel() override;
			void InsertLabel(const StringView& name) override;
			void End() override;
			void IssueBarriers(const GpuBarriers& barriers) override;

			/** Returns an unique identifier of this command buffer. */
			u32 GetId() const { return mId; }

			/** Returns the thread that the command buffer is allowed to be used on. */
			ThreadId GetOwnerThread() const { return mOwnerThread; }

			/**
			 * Prepares the command buffer to be submitted on a GpuQueue.
			 *
			 * @param queueType			Usage of the queue the command buffer will be submitted on.
			 * @param queueIndex			Index of the queue the command buffer will be submitted on.
			 * @return						Information required for submitting the command buffer on the queue.
			 * 
			 * @note Submit thread only.
			 */
			GpuCommandBufferSubmitInformation PrepareForSubmitOnSubmitThread(GpuQueueType queueType, u32 queueIndex);

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
			bool IsSubmitted() const { return mState == GpuCommandBufferState::Executing; }

			/** Returns true if the command buffer is currently recording (but not within a render pass). */
			bool IsRecording() const { return mState == GpuCommandBufferState::Recording || mState == GpuCommandBufferState::RecordingRenderPass; }

			/** Returns true if the command buffer is ready to be submitted to a queue. */
			bool IsReadyForSubmit() const { return mState == GpuCommandBufferState::RecordingDone; }

			/** Returns true if the command buffer is done executing on the device. */
			bool IsDone() const { return mState == GpuCommandBufferState::Done; }

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

			/** Notifies the command buffer that the pool it was allocated from has been reset. */
			void NotifyParentPoolReset();

			/** @copydoc GpuCommandBuffer::Cleanup */
			void Cleanup() override;

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
			friend class VulkanGpuCommandBufferPool;
			friend class VulkanGpuCommandBuffer;
			friend class VulkanGpuQueue;
			friend class VulkanGpuBuffer;
			friend class VulkanTexture;
			friend class VulkanBarrierHelper;

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

			/** Cached data from GpuParameterSet that were registered during render pass start. */
			struct CachedGpuParameterData
			{
				VkDescriptorSet DescriptorSet;
				TInlineArray<u32, 4> DynamicOffsets;
			};

			VulkanGpuCommandBuffer(VulkanGpuDevice& device, VulkanGpuCommandBufferPool& pool, u32 id, VkCommandBuffer commandBufferHandle, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation);

			/** Returns the pool the command buffer was allocated from. */
			VulkanGpuCommandBufferPool& GetPool() const { return mPool; }

			/** Makes the command buffer ready to start recording commands. */
			void Begin();

			/** Begins render pass recording. Must be called within begin()/end() calls. */
			void BeginRenderPass();

			/** Checks if all the prerequisites for rendering have been made (e.g. render target and pipeline state are set.) */
			bool IsReadyForRender();

			/** Marks the command buffer as submitted on a queue. */
			void SetIsSubmitted() { mState = GpuCommandBufferState::Executing; }

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

			/** Binds the currently stored GPU parameter sets, if dirty. */
			void BindGpuParameters(const SPtr<GpuPipelineParameterLayout>& pipelineParameterLayout, VulkanBarrierHelper& barrierHelper);

			/** Creates an array of clear values from the specified clear mask and values. To be used for the explicit clear command, or render bass begin. */
			Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1> BuildClearValues(RenderSurfaceMask clearMask, const Color& color, float depth, u16 stencil);

			/**
			 * Executes a clear command in the command buffer.
			 *
			 * @param area			Area in the currently bound render target to clear.
			 * @param clearMask		Mask specifying which surfaces of the currently bound render target to clear.
			 * @param color			Color value used for clearing color attachments.
			 * @param depth			Depth value used for clearing the depth attachment.
			 * @param stencil		Stencil value used for clearing the stencil attachment.
			 */
			void ClearAttachments(const Area2I& area, RenderSurfaceMask clearMask, const Color& color, float depth, u16 stencil);

			/**
			 * Executes a clear command in the command buffer.
			 *
			 * @param area			Area in the currently bound render target to clear.
			 * @param clearMask		Mask specifying which surfaces of the currently bound render target to clear.
			 * @param clearValues	Values used for clearing attachments.
			 */
			void ClearAttachments(const Area2I& area, RenderSurfaceMask clearMask, const Array<VkClearValue, B3D_MAXIMUM_RENDER_TARGET_COUNT + 1>& clearValues);

			/** Returns the current viewport area in pixels. This depends on the currently bound framebuffer and normalized viewport area. */
			Area2I GetViewportArea() const;

			/** Returns the current area of the render pass in pixels. This depends on the currently bound framebuffer. */
			Area2I GetRenderPassArea() const;

			/** Notifies the active render target that a rendering command was queued that will potentially change its contents. */
			void NotifyRenderTargetModified();

			/** Returns the owner GPU device, cast as a VulkanGpuDevice. */
			VulkanGpuDevice& GetVulkanGpuDevice() const { return static_cast<VulkanGpuDevice&>(mGpuDevice); }

			/** Rebuilds the flat dynamic offset array from per-set arrays. */
			void RebuildFlatDynamicOffsets();

			u32 mId;
			VkCommandBuffer mCommandBufferHandle;
			VulkanGpuCommandBufferPool& mPool;
			VkFence mFence;
			ThreadId mOwnerThread;

			VulkanSemaphore* mIntraQueueSemaphore = nullptr;
			VulkanSemaphore* mInterQueueSemaphores[B3D_MAX_COMMAND_BUFFER_DEPENDENCIES]{};
			mutable u32 mNumUsedInterQueueSemaphores = 0;

			VulkanFramebuffer* mFramebuffer = nullptr;
			RenderSurfaceMask mRenderTargetReadOnlyMask = RT_NONE;

			VulkanResourceTracker mResourceTracker;
			VulkanBarrierHelper mBarrierHelper;
			GpuQueueId mSubmittedQueueId;

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
			bool mIsDebugLabelOpen = false;
			DescriptorSetBindFlags mDescriptorSetsBindState;
			TInlineArray<SPtr<VulkanGpuParameterSet>, 4> mBoundGpuParameterSets;

			VkBuffer mVertexBuffersTemp[B3D_MAX_BOUND_VERTEX_BUFFERS]{};
			VkDeviceSize mVertexBufferOffsetsTemp[B3D_MAX_BOUND_VERTEX_BUFFERS]{};
			VkDescriptorSet* mDescriptorSetsTemp;
			TransitionInfo mTransitionInfoTemp[GQT_COUNT];
			Vector<VulkanEvent*> mQueuedEvents;
			Vector<IVulkanRenderWindowSurface*> mAcquiredSurfaces;
			TInlineArray<TInlineArray<u32, 4>, 4> mDynamicOffsetsPerSet;
			TInlineArray<UnorderedMap<u32, u32>, 4> mDynamicOffsetsOverridesPerSet;
			TInlineArray<u32, 16> mFlatDynamicOffsets;
			UnorderedMap<GpuParameterSet*, CachedGpuParameterData> mRenderPassGpuParameterSetCache;

			SPtr<RenderTarget> mRenderTarget;
			bool mRenderTargetModified = false;

#if B3D_BUILD_TYPE_DEVELOPMENT
			Vector<QueryInformation> mOpenQueries; // Only used for validation
#endif
		};

		/** @} */
	} // namespace render
} // namespace b3d
