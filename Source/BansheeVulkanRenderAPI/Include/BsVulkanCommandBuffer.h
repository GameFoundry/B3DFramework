//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsCommandBuffer.h"
#include "BsVulkanRenderAPI.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class LVulkanCommandBuffer;

#define BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE 32

	/** Pool that allocates and distributes Vulkan command buffers. */
	class CommandBufferPool
	{
	public:
		CommandBufferPool(VulkanDevice& device);
		~CommandBufferPool();

		/** Attempts to find a free command buffer, or creates a new one if not found. */
		LVulkanCommandBuffer* getBuffer(CommandBufferType type, UINT32 queueIdx, bool secondary);

	private:
		/** Creates a new command buffer. */
		LVulkanCommandBuffer* createBuffer(VulkanQueueType type, bool secondary);

		/** Returns a Vulkan command pool for the specified queue type. */
		VkCommandPool getPool(VulkanQueueType type);

		VulkanDevice& mDevice;
		VkCommandPool mPools[VQT_COUNT];

		LVulkanCommandBuffer* mBuffers[VQT_COUNT][BS_MAX_QUEUES_PER_TYPE][BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE];
	};

	/** 
	 * Represents a direct wrapper over an internal Vulkan command buffer. This is unlike VulkanCommandBuffer which is a
	 * higher level class, and it allows for re-use by internally using multiple low-level command buffers.
	 */
	class LVulkanCommandBuffer
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
		LVulkanCommandBuffer(VulkanDevice& device, VkCommandPool pool, bool secondary);
		~LVulkanCommandBuffer();

		/** Makes the command buffer ready to start recording commands. */
		void begin();

		/** Ends command buffer command recording (as started with begin()). */
		void end();

		/** Begins render pass recording. Must be called within begin()/end() calls. */
		void beginRenderPass();

		/** Ends render pass recording (as started with beginRenderPass(). */
		void endRenderPass();

		/** Returns the handle to the internal Vulkan command buffer wrapped by this object. */
		VkCommandBuffer getHandle() const { return mCmdBuffer; }

		/** Returns a fence that can be used for tracking when the command buffer is done executing. */
		VkFence getFence() const { return mFence; }

		/** 
		 * Returns a semaphore that may be used for synchronizing execution between command buffers executing on different 
		 * queues. 
		 */
		VkSemaphore getSemaphore() const { return mSemaphore; }

		/** Returns a counter that gets incremented whenever the command buffer is done executing. */
		UINT32 getFenceCounter() const { return mFenceCounter; }

		/** Checks the internal fence and changes command buffer state if done executing. */
		void refreshFenceStatus();

	private:
		friend class CommandBufferPool;

		State mState;
		VulkanDevice& mDevice;
		VkCommandPool mPool;
		VkCommandBuffer mCmdBuffer;
		VkFence mFence;
		VkSemaphore mSemaphore;
		UINT32 mFenceCounter;
	};

	/** CommandBuffer implementation for Vulkan. */
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		/** 
		 * Returns the handle to the internal command buffer. This is a lower-level command buffer that more directly
		 * maps to Vulkan's command buffers.
		 */
		LVulkanCommandBuffer& getBuffer() const { return *mBuffer; }

		/** 
		 * Tasks the command buffer to find a new internal command buffer. Call this after the command buffer has been
		 * submitted to a queue (it's not allowed to be used until the queue is done with it).
		 */
		void acquireNewBuffer();

	private:
		friend class VulkanCommandBufferManager;

		VulkanCommandBuffer(VulkanDevice& device, UINT32 id, CommandBufferType type, UINT32 queueIdx, 
			bool secondary);

		LVulkanCommandBuffer* mBuffer;
		VulkanDevice& mDevice;
	};

	/** @} */
}