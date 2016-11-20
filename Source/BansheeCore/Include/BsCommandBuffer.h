//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Mask that determines synchronization between command buffers executing on different hardware queues. */
	class BS_CORE_EXPORT CommandSyncMask
	{
	public:
		/** 
		 * Registers a dependency on a command buffer. Use getMask() to get the new mask value after registering all 
		 * dependencies.
		 */
		void addDependency(const SPtr<CommandBuffer>& buffer);

		/** Returns a combined mask that contains all the required dependencies. */
		UINT32 getMask() const { return mMask; }

		/** Uses the queue type and index to generate a mask with a bit set for that queue's global index. */
		static UINT32 getGlobalQueueMask(GpuQueueType type, UINT32 queueIdx);

		/** Uses the queue type and index to generate a global queue index. */
		static UINT32 getGlobalQueueIdx(GpuQueueType type, UINT32 queueIdx);

		/** Uses the global queue index to retrieve local queue index and queue type. */
		static UINT32 getQueueIdxAndType(UINT32 globalQueueIdx, GpuQueueType& type);

	private:
		UINT32 mMask = 0;
	};

	/** 
	 * Contains a list of render API commands that can be queued for execution on the GPU. User is allowed to populate the
	 * command buffer from any thread, ensuring render API command generation can be multi-threaded. Command buffers
	 * must always be created on the core thread. Same command buffer cannot be used on multiple threads simulateously
	 * without external synchronization.
	 */
	class BS_CORE_EXPORT CommandBuffer
	{
	public:
		virtual ~CommandBuffer() { }

		/**
		 * Creates a new CommandBuffer.
		 * 
		 * @param[in]	type		Determines what type of commands can be added to the command buffer.
		 * @param[in]	deviceIdx	Index of the GPU the command buffer will be used to queue commands on. 0 is always
		 *							the primary available GPU.
		 * @param[in]	queueIdx	Index of the hardware queue the command buffer will be used on. Command buffers with
		 *							the same index will execute sequentially, but command buffers with different queue
		 *							indices may execute in parallel, for a potential performance improvement. Queue indices
		 *							are unique per buffer type (e.g. upload index 0 and graphics index 0 may map to 
		 *							different queues internally). Must be in range [0, 7].
		 * @param[in]	secondary	If true the command buffer will not be allowed to execute on its own, but it can
		 *							be appended to a primary command buffer. 
		 * @return					New CommandBuffer instance.
		 * 
		 * @note The parallelism provided by @p queueIdx is parallelism on the GPU itself, it has nothing to do with CPU
		 *		 parallelism or threads.
		 */
		static SPtr<CommandBuffer> create(GpuQueueType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false);

		/** Returns the type of queue the command buffer will execute on. */
		GpuQueueType getType() const { return mType; }

		/** Returns the index of the queue the command buffer will execute on. */
		UINT32 getQueueIdx() const { return mQueueIdx; }

		/** Returns the device index this buffer will execute on. */
		UINT32 getDeviceIdx() const { return mDeviceIdx; }

	protected:
		CommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary);

		GpuQueueType mType;
		UINT32 mDeviceIdx;
		UINT32 mQueueIdx;
		bool mIsSecondary;
	};

	/** @} */
}