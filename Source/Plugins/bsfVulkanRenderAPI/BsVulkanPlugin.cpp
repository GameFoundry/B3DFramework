//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanPrerequisites.h"
#include "Managers/BsVulkanGpuBackendFactory.h"

using namespace bs;

extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return ct::VulkanGpuBackendFactory::SystemName;
}
