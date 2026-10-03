//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalPrerequisites.h"
#include "B3DMetalGpuBackendFactory.h"

using namespace b3d;

extern "C" void* LoadPlugin_bsfMetalGpuBackend()
{
	return static_cast<void*>(B3DNew<render::MetalGpuBackendFactory>());
}

extern "C" void UnloadPlugin_bsfMetalGpuBackend(void* instance)
{
	B3DDelete(static_cast<GpuBackendFactory*>(instance));
}

#if !B3D_MONOLITHIC_BUILD
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return render::MetalGpuBackendFactory::SystemName;
}

extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfMetalGpuBackend();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(render::MetalGpuBackendFactory* instance)
{
	UnloadPlugin_bsfMetalGpuBackend(instance);
}
#endif
