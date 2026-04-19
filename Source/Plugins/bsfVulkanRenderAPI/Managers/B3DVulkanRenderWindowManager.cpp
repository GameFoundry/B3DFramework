//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DVulkanRenderWindowManager.h"
#include "B3DVulkanRenderWindowSurface.h"
#include "B3DVulkanHeadlessRenderWindowSurface.h"

using namespace b3d;

SPtr<render::IRenderWindowSurface> VulkanRenderWindowManager::CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation)
{
	if(createInformation.Headless)
		return B3DMakeShared<render::VulkanHeadlessRenderWindowSurface>(createInformation);

	return B3DMakeShared<render::VulkanRenderWindowSurface>(createInformation);
}

