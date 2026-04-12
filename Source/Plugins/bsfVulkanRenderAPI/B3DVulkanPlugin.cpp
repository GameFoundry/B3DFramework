//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanPrerequisites.h"
#include "Managers/B3DVulkanGpuBackendFactory.h"

using namespace b3d;

extern "C" void* LoadPlugin_bsfVulkanRenderAPI()
{
	return static_cast<void*>(B3DNew<render::VulkanGpuBackendFactory>());
}

extern "C" void UnloadPlugin_bsfVulkanRenderAPI(void* instance)
{
	B3DDelete(static_cast<GpuBackendFactory*>(instance));
}

extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return render::VulkanGpuBackendFactory::SystemName;
}

extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfVulkanRenderAPI();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(render::VulkanGpuBackendFactory* instance)
{
	UnloadPlugin_bsfVulkanRenderAPI(instance);
}
