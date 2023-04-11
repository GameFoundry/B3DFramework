//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Handles creation of command buffers. See CommandBuffer.
		 *
		 * @note Core thread only.
		 */
		class B3D_CORE_EXPORT CommandBufferManager : public Module<CommandBufferManager>
		{
		public:
			virtual ~CommandBufferManager() = default;

			/** @copydoc CommandBuffer::Create */
			SPtr<CommandBuffer> Create(GpuQueueType queueType);

		protected:
			friend CommandBuffer;

			/** Creates a command buffer with the specified ID. See create(). */
			virtual SPtr<CommandBuffer> CreateInternal(GpuQueueType queueType) = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
