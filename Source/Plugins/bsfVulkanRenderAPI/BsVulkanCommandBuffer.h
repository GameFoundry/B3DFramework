//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanResource.h"
#include "BsVulkanGpuPipelineState.h"

namespace bs { namespace ct
{
	class VulkanOcclusionQuery;
	class VulkanTimerQuery;
	class VulkanImage;

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
		VkSemaphore getHandle() const { return mSemaphore; }

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
		VulkanCmdBuffer* getBuffer(UINT32 queueFamily, bool secondary);

	private:
		/** Command buffer pool and related information. */
		struct PoolInfo
		{
			VkCommandPool pool = VK_NULL_HANDLE;
			VulkanCmdBuffer* buffers[BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY];
			UINT32 queueFamily = -1;
		};

		/** Creates a new command buffer. */
		VulkanCmdBuffer* createBuffer(UINT32 queueFamily, bool secondary);

		VulkanDevice& mDevice;
		UnorderedMap<UINT32, PoolInfo> mPools;
		UINT32 mNextId;
	};

	/** Determines where are the current descriptor sets bound to. */
	enum class DescriptorSetBindFlag
	{
		None = 0,
		Graphics = 1 << 0,
		Compute = 1 << 1
	};

	/** Specifies for what purpose is a resource being bound to a command buffer. */
	enum class ResourceUsage
	{
		ShaderBind,
		Framebuffer,
		Transfer
	};

	typedef Flags<DescriptorSetBindFlag> DescriptorSetBindFlags;
	BS_FLAGS_OPERATORS(DescriptorSetBindFlag)

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
			Submitted
		};

	public:
		VulkanCmdBuffer(VulkanDevice& device, UINT32 id, VkCommandPool pool, UINT32 queueFamily, bool secondary);
		~VulkanCmdBuffer();

		/** Returns an unique identifier of this command buffer. */
		UINT32 getId() const { return mId; }

		/** Returns the index of the queue family this command buffer is executing on. */
		UINT32 getQueueFamily() const { return mQueueFamily; }

		/** Returns the index of the device this command buffer will execute on. */
		UINT32 getDeviceIdx() const;

		/** Makes the command buffer ready to start recording commands. */
		void begin();

		/** Ends command buffer command recording (as started with begin()). */
		void end();

		/** Begins render pass recording. Must be called within begin()/end() calls. */
		void beginRenderPass();

		/** Ends render pass recording (as started with beginRenderPass(). */
		void endRenderPass();

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
		void submit(VulkanQueue* queue, UINT32 queueIdx, UINT32 syncMask);

		/** Returns the handle to the internal Vulkan command buffer wrapped by this object. */
		VkCommandBuffer getHandle() const { return mCmdBuffer; }

		/** Returns a fence that can be used for tracking when the command buffer is done executing. */
		VkFence getFence() const { return mFence; }

		/** 
		 * Returns a semaphore that may be used for synchronizing execution between command buffers executing on the same
		 * queue.
		 */
		VulkanSemaphore* getIntraQueueSemaphore() const { return mIntraQueueSemaphore; }

		/** 
		 * Returns a semaphore that may be used for synchronizing execution between command buffers executing on different
		 * queues. Note that these semaphores get used each time they are requested, and there is only a fixed number
		 * available. If all are used up, null will be returned. New semaphores are generated when allocateSemaphores()
		 * is called.
		 */
		VulkanSemaphore* requestInterQueueSemaphore() const;

		/** 
		 * Allocates a new set of semaphores that may be used for synchronizing execution between different command buffers.
		 * Releases the previously allocated semaphores, if they exist. Use getIntraQueueSemaphore() & 
		 * requestInterQueueSemaphore() to retrieve latest allocated semaphores.
		 * 
		 * @param[out]	semaphores	Output array to place all allocated semaphores in. The array must be of size
		 *							(BS_MAX_VULKAN_CB_DEPENDENCIES + 1).
		 */
		void allocateSemaphores(VkSemaphore* semaphores);

		/** Returns true if the command buffer is currently being processed by the device. */
		bool isSubmitted() const { return mState == State::Submitted; }

		/** Returns true if the command buffer is currently recording (but not within a render pass). */
		bool isRecording() const { return mState == State::Recording; }

		/** Returns true if the command buffer is ready to be submitted to a queue. */
		bool isReadyForSubmit() const { return mState == State::RecordingDone; }

		/** Returns true if the command buffer is currently recording a render pass. */
		bool isInRenderPass() const { return mState == State::RecordingRenderPass; }

		/** 
		 * Checks the internal fence if done executing. 
		 * 
		 * @param[in]	block	If true, the system will block until the fence is signaled.
		 */
		bool checkFenceStatus(bool block) const;

		/** 
		 * Resets the command buffer back in Ready state. Should be called when command buffer is done executing on a 
		 * queue. 
		 */
		void reset();

		/** 
		 * Lets the command buffer know that the provided resource has been queued on it, and will be used by the
		 * device when the command buffer is submitted. If a resource is an image or a buffer use the more specific
		 * registerResource() overload.
		 */
		void registerResource(VulkanResource* res, VulkanUseFlags flags);

		/** 
		 * Lets the command buffer know that the provided image resource has been queued on it, and will be used by the
		 * device when the command buffer is submitted. Executes a layout transition to @p newLayout (if needed), and
		 * updates the externally visible image layout field to @p finalLayout (once submitted).
		 * 
		 * @param[in]	res						Image to register with the command buffer.
		 * @param[in]	range					Range of sub-resources to register.
		 * @param[in]	newLayout				Layout the image needs to be transitioned in before use. Set to undefined
		 *										layout if no transition is required.
		 * @param[in]	finalLayout				Determines what value the externally visible image layout will be set after
		 *										submit() is called. Normally this will be same as @p newLayout, but can be
		 *										different if some form of automatic layout transitions are happening.
		 * @param[in]	flags					Flags that determine how will be command buffer be using the buffer.
		 * @param[in]	usage					Determines for what purpose is the resource being registered for.
		 */
		void registerResource(VulkanImage* res, const VkImageSubresourceRange& range, VkImageLayout newLayout, 
							  VkImageLayout finalLayout, VulkanUseFlags flags, ResourceUsage usage);

		/** 
		 * Lets the command buffer know that the provided image resource has been queued on it, and will be used by the
		 * device when the command buffer is submitted. Assumes the image is in its optimal layout.
		 */
		void registerResource(VulkanImage* res, const VkImageSubresourceRange& range, VulkanUseFlags flags, 
			ResourceUsage usage);

		/** 
		 * Lets the command buffer know that the provided image resource has been queued on it, and will be used by the
		 * device when the command buffer is submitted. 
		 */
		void registerResource(VulkanBuffer* res, VkAccessFlags accessFlags, VulkanUseFlags flags);

		/** 
		 * Lets the command buffer know that the provided framebuffer resource has been queued on it, and will be used by
		 * the device when the command buffer is submitted.
		 */
		void registerResource(VulkanFramebuffer* res, RenderSurfaceMask loadMask, UINT32 readMask);

		/** Notifies the command buffer that the provided query has been queued on it. */
		void registerQuery(VulkanOcclusionQuery* query) { mOcclusionQueries.insert(query); }

		/** Notifies the command buffer that the provided query has been queued on it. */
		void registerQuery(VulkanTimerQuery* query) { mTimerQueries.insert(query); }

		/************************************************************************/
		/* 								COMMANDS	                     		*/
		/************************************************************************/

		/** 
		 * Assigns a render target the the command buffer. This render target's framebuffer and render pass will be used
		 * when beginRenderPass() is called. Command buffer must not be currently recording a render pass.
		 */
		void setRenderTarget(const SPtr<RenderTarget>& rt, UINT32 readOnlyFlags, RenderSurfaceMask loadMask);

		/** Clears the entirety currently bound render target. */
		void clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask);

		/** Clears the viewport portion of the currently bound render target. */
		void clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask);

		/** Assigns a pipeline state to use for subsequent draw commands. */
		void setPipelineState(const SPtr<GraphicsPipelineState>& state);

		/** Assigns a pipeline state to use for subsequent dispatch commands. */
		void setPipelineState(const SPtr<ComputePipelineState>& state);

		/** Assign GPU params to the GPU programs bound by the pipeline state. */
		void setGpuParams(const SPtr<GpuParams>& gpuParams);

		/** Sets the current viewport which determine to which portion of the render target to render to. */
		void setViewport(const Rect2& area);

		/** 
		 * Sets the scissor rectangle area which determines in which area if the viewport are the fragments allowed to be
		 * generated. Only relevant if enabled on the pipeline state.
		 */
		void setScissorRect(const Rect2I& area);

		/** Sets a stencil reference value that will be used for comparisons in stencil operations, if enabled. */
		void setStencilRef(UINT32 value);

		/** Changes how are primitives interpreted as during rendering. */
		void setDrawOp(DrawOperationType drawOp);

		/** Sets one or multiple vertex buffers that will be used for subsequent draw() or drawIndexed() calls. */
		void setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers);

		/** Sets an index buffer that will be used for subsequent drawIndexed() calls. */
		void setIndexBuffer(const SPtr<IndexBuffer>& buffer);

		/** Sets a declaration that determines how are vertex buffer contents interpreted. */
		void setVertexDeclaration(const SPtr<VertexDeclaration>& decl);

		/** Executes a draw command using the currently bound graphics pipeline, vertex buffer and render target. */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount);

		/** Executes a draw command using the currently bound graphics pipeline, index & vertex buffer and render target. */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 instanceCount);

		/** Executes a dispatch command using the currently bound compute pipeline. */
		void dispatch(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ);

		/** 
		 * Registers a command that signals the event when executed. Will be delayed until the end of the current
		 * render pass, if any.
		 */
		void setEvent(VulkanEvent* event);

		/** 
		 * Registers a command that resets the query. The command will be delayed until the next submit() if a render
		 * pass is currently in progress, but is guaranteed to execute before this command buffer is submitted.
		 */
		void resetQuery(VulkanQuery* query);

		/** 
		 * Issues a pipeline barrier on the provided buffer. See vkCmdPipelineBarrier in Vulkan spec. for usage
		 * information.
		 */
		void memoryBarrier(VkBuffer buffer, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags,
						   VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

		/** 
		 * Issues a pipeline barrier on the provided image. See vkCmdPipelineBarrier in Vulkan spec. for usage
		 * information.
		 */
		void memoryBarrier(VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags,
						   VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImageLayout layout, 
						   const VkImageSubresourceRange& range);

		/** 
		 * Issues a pipeline barrier on the provided image, changing its layout. See vkCmdPipelineBarrier in Vulkan spec. 
		 * for usage information.
		 */
		void setLayout(VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, 
			VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& range);

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
		VkImageLayout getCurrentLayout(VulkanImage* image, const VkImageSubresourceRange& range, bool inRenderPass);

	private:
		friend class VulkanCmdBufferPool;
		friend class VulkanCommandBuffer;
		friend class VulkanQueue;

		/** Contains information about a single Vulkan resource bound/used on this command buffer. */
		struct ResourceUseHandle
		{
			bool used;
			VulkanUseFlags flags;
		};

		/** Contains information about a single Vulkan buffer resource bound/used on this command buffer. */
		struct BufferInfo
		{
			VkAccessFlags accessFlags;
			ResourceUseHandle useHandle;

			/** 
			 * True if the buffer was at some point written to by the shader during the current render pass, and barrier
			 * wasn't issued yet. 
			 */
			bool needsBarrier;
		};

		/** Contains information about a single Vulkan image resource bound/used on this command buffer. */
		struct ImageInfo
		{
			ResourceUseHandle useHandle;

			UINT32 subresourceInfoIdx;
			UINT32 numSubresourceInfos;
		};

		/** Contains information about a range of Vulkan image sub-resources bound/used on this command buffer. */
		struct ImageSubresourceInfo
		{
			VkImageSubresourceRange range;

			// Only relevant for layout transitions
			VkImageLayout initialLayout;
			VkImageLayout currentLayout;
			VkImageLayout requiredLayout;
			VkImageLayout finalLayout;

			bool isFBAttachment : 1;
			bool isShaderInput : 1;
			bool hasTransitioned : 1;
			bool hasExternalTransition : 1;
			bool isReadOnly : 1;
			bool isInitialReadOnly : 1;

			/** 
			 * True if the buffer was at some point written to by the shader during the current render pass, and barrier
			 * wasn't issued yet. 
			 */
			bool needsBarrier : 1;
		};

		/** Checks if all the prerequisites for rendering have been made (e.g. render target and pipeline state are set.) */
		bool isReadyForRender();

		/** Marks the command buffer as submitted on a queue. */
		void setIsSubmitted() { mState = State::Submitted; }

		/** Binds the current graphics pipeline to the command buffer. Returns true if bind was successful. */
		bool bindGraphicsPipeline();

		/** 
		 * Binds any dynamic states to the pipeline, as required. 
		 *
		 * @param[in]	forceAll	If true all states will be bound. If false only states marked as dirty will be bound.
		 */
		void bindDynamicStates(bool forceAll);

		/** Binds the currently stored GPU parameters object, if dirty. */
		void bindGpuParams();

		/** Clears the specified area of the currently bound render target. */
		void clearViewport(const Rect2I& area, UINT32 buffers, const Color& color, float depth, UINT16 stencil, 
			UINT8 targetMask);

		/** Starts and ends a render pass, intended only for a clear operation. */
		void executeClearPass();

		/** Executes any queued layout transitions by issuing a pipeline barrier. */
		void executeLayoutTransitions();

		/** 
		 * Updates final layouts for images used by the current framebuffer, reflecting layout changes performed by render
		 * pass' automatic layout transitions. 
		 */
		void updateFinalLayouts();

		/** 
		 * Updates an existing sub-resource info range with new layout, use flags and framebuffer flag. Returns true if
		 * the bound sub-resource is a read-only framebuffer attachment.
		 */
		bool updateSubresourceInfo(VulkanImage* image, UINT32 imageInfoIdx, ImageSubresourceInfo& subresourceInfo, 
			VkImageLayout newLayout, VkImageLayout finalLayout, VulkanUseFlags flags, ResourceUsage usage);

		/** Finds a subresource info structure containing the specified face and mip level of the provided image. */
		ImageSubresourceInfo& findSubresourceInfo(VulkanImage* image, UINT32 face, UINT32 mip);

		/** Gets all queries registered on this command buffer that haven't been ended. */
		void getInProgressQueries(Vector<VulkanTimerQuery*>& timer, Vector<VulkanOcclusionQuery*>& occlusion) const;

		/** Returns the read mask for the current framebuffer. */
		RenderSurfaceMask getFBReadMask();

		UINT32 mId;
		UINT32 mQueueFamily;
		State mState;
		VulkanDevice& mDevice;
		VkCommandPool mPool;
		VkCommandBuffer mCmdBuffer;
		VkFence mFence;

		VulkanSemaphore* mIntraQueueSemaphore;
		VulkanSemaphore* mInterQueueSemaphores[BS_MAX_VULKAN_CB_DEPENDENCIES];
		mutable UINT32 mNumUsedInterQueueSemaphores;

		VulkanFramebuffer* mFramebuffer;
		UINT32 mRenderTargetReadOnlyFlags;
		RenderSurfaceMask mRenderTargetLoadMask;

		UnorderedMap<VulkanResource*, ResourceUseHandle> mResources;
		UnorderedMap<VulkanResource*, UINT32> mImages;
		UnorderedMap<VulkanResource*, BufferInfo> mBuffers;
		UnorderedSet<VulkanOcclusionQuery*> mOcclusionQueries;
		UnorderedSet<VulkanTimerQuery*> mTimerQueries;
		Vector<ImageInfo> mImageInfos;
		Vector<ImageSubresourceInfo> mSubresourceInfoStorage;
		Set<UINT32> mPassTouchedSubresourceInfos; // All subresource infos touched by the current render pass
		UINT32 mGlobalQueueIdx;

		SPtr<VulkanGraphicsPipelineState> mGraphicsPipeline;
		SPtr<VulkanComputePipelineState> mComputePipeline;
		SPtr<VertexDeclaration> mVertexDecl;
		Rect2 mViewport;
		Rect2I mScissor;
		UINT32 mStencilRef;
		DrawOperationType mDrawOp;
		UINT32 mNumBoundDescriptorSets;
		bool mGfxPipelineRequiresBind : 1;
		bool mCmpPipelineRequiresBind : 1;
		bool mViewportRequiresBind : 1;
		bool mStencilRefRequiresBind : 1;
		bool mScissorRequiresBind : 1;
		bool mBoundParamsDirty : 1;
		DescriptorSetBindFlags mDescriptorSetsBindState;
		SPtr<VulkanGpuParams> mBoundParams;

		std::array<VkClearValue, BS_MAX_MULTIPLE_RENDER_TARGETS + 1> mClearValues;
		ClearMask mClearMask;
		Rect2I mClearArea;

		Vector<VulkanSemaphore*> mSemaphoresTemp;
		VkBuffer mVertexBuffersTemp[BS_MAX_BOUND_VERTEX_BUFFERS];
		VkDeviceSize mVertexBufferOffsetsTemp[BS_MAX_BOUND_VERTEX_BUFFERS];
		VkDescriptorSet* mDescriptorSetsTemp;
		UnorderedMap<UINT32, TransitionInfo> mTransitionInfoTemp;
		Vector<VkImageMemoryBarrier> mLayoutTransitionBarriersTemp;
		UnorderedMap<VulkanImage*, UINT32> mQueuedLayoutTransitions;
		Vector<VulkanEvent*> mQueuedEvents;
		Vector<VulkanQuery*> mQueuedQueryResets;
		UnorderedSet<VulkanSwapChain*> mSwapChains;
	};

	/** CommandBuffer implementation for Vulkan. */
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		/** 
		 * Submits the command buffer for execution. 
		 * 
		 * @param[in]	syncMask	Mask that controls which other command buffers does this command buffer depend upon
		 *							(if any). See description of @p syncMask parameter in RenderAPI::executeCommands().
		 */
		void submit(UINT32 syncMask);

		/** 
		 * Returns the internal command buffer. 
		 * 
		 * @note	This buffer will change after a submit() call.
		 */
		VulkanCmdBuffer* getInternal() const { return mBuffer; }

	private:
		friend class VulkanCommandBufferManager;

		VulkanCommandBuffer(VulkanDevice& device, GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx,
			bool secondary);

		~VulkanCommandBuffer();

		/** 
		 * Tasks the command buffer to find a new internal command buffer. Call this after the command buffer has been
		 * submitted to a queue (it's not allowed to be used until the queue is done with it).
		 */
		void acquireNewBuffer();

		VulkanCmdBuffer* mBuffer;
		VulkanDevice& mDevice;
		VulkanQueue* mQueue;
		UINT32 mIdMask;
	};

	/** @} */
}}