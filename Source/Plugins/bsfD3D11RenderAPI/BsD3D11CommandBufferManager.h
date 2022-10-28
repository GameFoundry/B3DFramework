//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Managers/BsCommandBufferManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**
		 * Handles creation of DirectX 11 command buffers. See CommandBuffer.
		 *
		 * @note Core thread only.
		 */
		class D3D11CommandBufferManager : public CommandBufferManager
		{
		public:
			SPtr<CommandBuffer> CreateInternal(GpuQueueType type, u32 deviceIdx = 0, u32 queueIdx = 0, bool secondary = false) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
