//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderAPI
		 *  @{
		 */

		/** Mask that determines synchronization between command buffers executing on different hardware queues. */
		class B3D_CORE_EXPORT CommandSyncMask
		{
		public:
			/**
			 * Registers a dependency on a command buffer. Use getMask() to get the new mask value after registering all
			 * dependencies.
			 */
			void AddDependency(const SPtr<CommandBuffer>& buffer);

			/** Returns a combined mask that contains all the required dependencies. */
			u32 GetMask() const { return mMask; }

			/** Uses the queue type and index to generate a mask with a bit set for that queue's global index. */
			static u32 GetGlobalQueueMask(GpuQueueType type, u32 queueIdx);

			/** Uses the queue type and index to generate a global queue index. */
			static u32 GetGlobalQueueIdx(GpuQueueType type, u32 queueIdx);

			/** Uses the global queue index to retrieve local queue index and queue type. */
			static u32 GetQueueIdxAndType(u32 globalQueueIdx, GpuQueueType& type);

		private:
			u32 mMask = 0;
		};

		/** Possible states that a CommandBuffer can be in. */
		enum class CommandBufferState
		{
			/** Command buffer doesn't have any commands recorded, nor has it been queued for execution. */
			Empty,

			/** Command buffer has one or multiple commands recorded, but they haven't been queued for execution. */
			Recording,

			/**
			 * Command buffer has been queued for execution, but still hasn't finished executing. Buffer that is
			 * executing cannot be modified or re-submitted for execution until done executing.
			 */
			Executing,

			/** Command buffer has been queued for execution and has finished executing. */
			Done
		};

		/**
		 * Contains a list of render API commands that can be queued for execution on the GPU. User is allowed to populate the
		 * command buffer from any thread, ensuring render API command generation can be multi-threaded. Command buffers
		 * must always be created on the core thread. Same command buffer cannot be used on multiple threads simulateously
		 * without external synchronization.
		 */
		class B3D_CORE_EXPORT CommandBuffer
		{
		public:
			virtual ~CommandBuffer();

			/**
			 * Creates a new CommandBuffer.
			 *
			 * @param[in]	type		Determines what type of commands can be added to the command buffer.
			 * @param[in]	deviceIdx	Index of the GPU the command buffer will be used to queue commands on. 0 is always
			 *							the primary available GPU.
			 * @param[in]	queueIdx	Index of the GPU queue the command buffer will be used on. Command buffers with
			 *							the same index will execute sequentially, but command buffers with different queue
			 *							indices may execute in parallel, for a potential performance improvement.
			 *
			 *							Caller must ensure to synchronize operations executing on different queues via
			 *							sync masks. Command buffer dependant on another command buffer should provide a sync
			 *							mask when being submitted (see RenderAPI::executeCommands).
			 *
			 *							Queue indices are unique per buffer type (e.g. upload index 0 and graphics index 0 may
			 *							map to different queues internally). Must be in range [0, 7].
			 * @param[in]	secondary	If true the command buffer will not be allowed to execute on its own, but it can
			 *							be appended to a primary command buffer.
			 * @return					New CommandBuffer instance.
			 */
			static SPtr<CommandBuffer> Create(GpuQueueType type, u32 deviceIdx = 0, u32 queueIdx = 0, bool secondary = false);

			/** Returns the type of queue the command buffer will execute on. */
			GpuQueueType GetType() const { return mType; }

			/** Returns the index of the queue the command buffer will execute on. */
			u32 GetQueueIdx() const { return mQueueIdx; }

			/** Returns the device index this buffer will execute on. */
			u32 GetDeviceIdx() const { return mDeviceIdx; }

			/** Returns the current state of the command buffer. */
			virtual CommandBufferState GetState() const = 0;

			/**
			 * Resets the command buffer back into initial state. Must only be used if the command buffer is
			 * not in the executing state.
			 */
			virtual void Reset() = 0;

			/** Triggers when the command buffer finishes execution on the GPU. */
			Event<void()> OnDidComplete;

			/** Triggered just before a command buffer is about to be destroyed. Provided parameters determines if the command buffer was ever submitted or not. */
			Event<void(bool)> OnDestroyed;

		protected:
			CommandBuffer(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary);

			GpuQueueType mType;
			u32 mDeviceIdx;
			u32 mQueueIdx;
			bool mIsSecondary;
			bool mIsSubmitted = false;
		};

		/** @} */
	} // namespace ct
} // namespace bs
