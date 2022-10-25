//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Managers/BsCommandBufferManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**
		 * Handles creation of OpenGL command buffers. See CommandBuffer.
		 *
		 * @note Core thread only.
		 */
		class GLCommandBufferManager : public CommandBufferManager
		{
		public:
			/** @copydoc CommandBufferManager::createInternal() */
			SPtr<CommandBuffer> CreateInternal(GpuQueueType type, u32 deviceIdx = 0, u32 queueIdx = 0, bool secondary = false);
		};

		/** @} */
	} // namespace ct
} // namespace bs
