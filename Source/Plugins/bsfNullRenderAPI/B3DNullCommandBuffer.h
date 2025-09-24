//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "Managers/BsCommandBufferManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup NullRenderAPI
		 *  @{
		 */

		/** Handles creation of Null command buffers. See CommandBuffer. */
		class NullCommandBufferManager final : public CommandBufferManager
		{
		public:
			SPtr<CommandBuffer> CreateInternal(GpuQueueType type, u32 deviceIdx = 0, u32 queueIdx = 0, bool secondary = false) override;
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
	} // namespace render
} // namespace b3d
