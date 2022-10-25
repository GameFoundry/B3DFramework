//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		VulkanRenderWindowManager() = default;

	protected:
		/** @copydoc RenderWindowManager::createImpl */
		SPtr<RenderWindow> CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow);
	};

	/** @} */
} // namespace bs
