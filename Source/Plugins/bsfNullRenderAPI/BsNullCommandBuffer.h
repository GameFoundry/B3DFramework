//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "RenderAPI/BsCommandBuffer.h"
#include "Managers/BsCommandBufferManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup NullRenderAPI
		 *  @{
		 */

		/** Handles creation of Null command buffers. See CommandBuffer. */
		class NullCommandBufferManager final : public CommandBufferManager
		{
		public:
			/** @copydoc CommandBufferManager::createInternal() */
			SPtr<CommandBuffer> CreateInternal(GpuQueueType type, u32 deviceIdx = 0, u32 queueIdx = 0, bool secondary = false);
		};

		/** Command buffer implementation for the null render backend. */
		class NullCommandBuffer final : public CommandBuffer
		{
		private:
			friend class NullCommandBufferManager;

			NullCommandBuffer(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
				: CommandBuffer(type, deviceIdx, queueIdx, secondary)
			{}

		public:
			CommandBufferState GetState() const { return CommandBufferState::Empty; }

			void Reset() {}
		};

		/** @} */
	} // namespace ct
} // namespace bs
