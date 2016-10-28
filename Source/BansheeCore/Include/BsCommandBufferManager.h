//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
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
		virtual ~CommandBufferManager() {}

		/** @copydoc CommandBuffer::create */
		SPtr<CommandBuffer> create(CommandBufferType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false);

	protected:
		friend CommandBuffer;

		/** Creates a command buffer with the specified ID. See create(). */
		virtual SPtr<CommandBuffer> createInternal(UINT32 id, CommandBufferType type, UINT32 deviceIdx = 0, 
			UINT32 queueIdx = 0, bool secondary = false) = 0;

		/** Called by a command buffer just before it is destroyed. */
		void notifyCommandBufferDestroyed(UINT32 id);

	private:
		bool mActiveCommandBuffers[BS_MAX_COMMAND_BUFFERS];
	};

	/** @} */
}