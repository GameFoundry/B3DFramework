//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Prerequisites.h"
#include "Managers/B3DD3D12GpuBackendFactory.h"

using namespace b3d;

extern "C" void* LoadPlugin_bsfD3D12RenderAPI()
{
	return static_cast<void*>(B3DNew<D3D12GpuBackendFactory>());
}

extern "C" void UnloadPlugin_bsfD3D12RenderAPI(void* instance)
{
	B3DDelete(static_cast<GpuBackendFactory*>(instance));
}

extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return D3D12GpuBackendFactory::SystemName;
}

extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfD3D12RenderAPI();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(D3D12GpuBackendFactory* instance)
{
	UnloadPlugin_bsfD3D12RenderAPI(instance);
}
