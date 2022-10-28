//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsCommandBuffer.h"
#include "BsGLRenderAPI.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**
		 * Command buffer implementation for OpenGL, which doesn't support multi-threaded command generation. Instead all
		 * commands are stored in an internal buffer, and then sent to the actual render API when the buffer is executed.
		 */
		class GLCommandBuffer : public CommandBuffer
		{
		public:
			/** Registers a new command in the command buffer. */
			void QueueCommand(const std::function<void()> command);

			/** Executes all commands in the command buffer. Not supported on secondary buffer. */
			void ExecuteCommands();

			CommandBufferState GetState() const override;
			void Reset() override;

		private:
			friend class GLCommandBufferManager;
			friend class GLRenderAPI;

			GLCommandBuffer(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary);

			/** Returns true if the command buffer has finished executing on the GPU. */
			bool IsComplete() const;

			GLsync mFence = 0;
			bool mCommandQueued = false;
			bool mIsSubmitted = false;
		};

		/** @} */
	} // namespace ct
} // namespace bs
