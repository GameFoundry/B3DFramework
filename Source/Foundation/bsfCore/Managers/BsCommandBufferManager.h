//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs { namespace ct 
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** 
	 * Handles creation of command buffers. See CommandBuffer. 
	 *
	 * @note Core thread only.
	 */
	class BS_CORE_EXPORT CommandBufferManager : public Module<CommandBufferManager>
	{
	public:
		virtual ~CommandBufferManager() = default;

		/** @copydoc CommandBuffer::create */
		SPtr<CommandBuffer> create(GpuQueueType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false);

	protected:
		friend CommandBuffer;

		/** Creates a command buffer with the specified ID. See create(). */
		virtual SPtr<CommandBuffer> createInternal(GpuQueueType type, UINT32 deviceIdx = 0,
			UINT32 queueIdx = 0, bool secondary = false) = 0;
	};

	/** @} */
}}