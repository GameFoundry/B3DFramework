//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPrerequisites.h"
#include "B3DNullGpuBackendFactory.h"

using namespace b3d;

extern "C" void* LoadPlugin_bsfNullRenderAPI()
{
	return static_cast<void*>(B3DNew<NullGpuBackendFactory>());
}

extern "C" void UnloadPlugin_bsfNullRenderAPI(void* instance)
{
	B3DDelete(static_cast<GpuBackendFactory*>(instance));
}

#if !B3D_MONOLITHIC_BUILD
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return NullGpuBackendFactory::SystemName;
}

extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfNullRenderAPI();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(NullGpuBackendFactory* instance)
{
	UnloadPlugin_bsfNullRenderAPI(instance);
}
#endif
