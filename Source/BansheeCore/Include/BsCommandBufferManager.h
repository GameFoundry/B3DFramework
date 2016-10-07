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
		/** @copydoc CommandBuffer::create */
		virtual SPtr<CommandBuffer> create(CommandBufferType type, UINT32 deviceIdx = 0, UINT32 syncMask = 0xFFFFFFFF, 
			bool secondary = false) = 0;
	};

	/** @} */
}