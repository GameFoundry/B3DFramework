//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsCommandBufferManager.h"

namespace BansheeEngine
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
		SPtr<CommandBuffer> createInternal(UINT32 id, GpuQueueType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false) override;
	};

	/** @} */
}