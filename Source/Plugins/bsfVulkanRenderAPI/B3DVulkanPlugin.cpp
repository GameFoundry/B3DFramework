//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanPrerequisites.h"
#include "Managers/B3DVulkanGpuBackendFactory.h"

using namespace b3d;

extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return render::VulkanGpuBackendFactory::SystemName;
}
