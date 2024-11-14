//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderWindowManager.h"
#include "BsVulkanRenderWindowSurface.h"

using namespace bs;

SPtr<ct::IRenderWindowSurface> VulkanRenderWindowManager::CreateRenderWindowSurface(const ct::RenderWindowSurfaceCreateInformation& createInformation)
{
	return B3DMakeShared<ct::VulkanRenderWindowSurface>(createInformation);
}

