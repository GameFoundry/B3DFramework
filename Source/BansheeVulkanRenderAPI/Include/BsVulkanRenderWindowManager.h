//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsRenderWindowManager.h"

namespace BansheeEngine
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

	/** @copydoc RenderWindowCoreManager */
	class VulkanRenderWindowCoreManager : public RenderWindowCoreManager
	{
	public:
		VulkanRenderWindowCoreManager(VulkanRenderAPI& renderAPI);

	protected:
		/** @copydoc RenderWindowCoreManager::createInternal */
		SPtr<RenderWindowCore> createInternal(RENDER_WINDOW_DESC& desc, UINT32 windowId) override;

		VulkanRenderAPI& mRenderAPI;
	};

	/** @} */
}