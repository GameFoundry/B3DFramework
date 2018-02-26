//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsRenderWindowManager.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc RenderWindowManager */
	class VulkanRenderWindowManager : public RenderWindowManager
	{
	public:
		VulkanRenderWindowManager();

	protected:
		/** @copydoc RenderWindowManager::createImpl */
		SPtr<RenderWindow> createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, const SPtr<RenderWindow>& parentWindow) override;
	};

	/** @} */
}