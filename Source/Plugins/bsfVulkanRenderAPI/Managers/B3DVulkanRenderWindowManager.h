//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsRenderWindowManager.h"

namespace b3d
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc RenderWindowManager */
	class VulkanRenderWindowManager : public RenderWindowManager
	{
	public:
		VulkanRenderWindowManager() = default;

		SPtr<render::IRenderWindowSurface> CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation) override;
	};

	/** @} */
} // namespace b3d
