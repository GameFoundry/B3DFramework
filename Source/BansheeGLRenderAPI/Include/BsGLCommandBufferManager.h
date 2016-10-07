//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsCommandBufferManager.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI-Internal
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
		/** @copydoc CommandBufferManager::create() */
		SPtr<CommandBuffer> create(CommandBufferType type, UINT32 deviceIdx = 0, UINT32 syncMask = 0xFFFFFFFF, 
			bool secondary = false) override;
	};

	/** @} */
}