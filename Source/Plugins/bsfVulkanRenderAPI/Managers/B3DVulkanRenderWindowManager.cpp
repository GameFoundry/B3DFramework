//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderWindowManager.h"
#include "BsVulkanRenderWindowSurface.h"

using namespace b3d;

SPtr<render::IRenderWindowSurface> VulkanRenderWindowManager::CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation)
{
	return B3DMakeShared<render::VulkanRenderWindowSurface>(createInformation);
}

