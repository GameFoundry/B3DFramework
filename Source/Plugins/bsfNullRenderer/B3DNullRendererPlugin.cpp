//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRendererPrerequisites.h"
#include "B3DNullRenderer.h"

using namespace b3d;

extern "C" void* LoadPlugin_bsfNullRenderer()
{
	return static_cast<void*>(B3DNew<NullRendererFactory>());
}

extern "C" void UnloadPlugin_bsfNullRenderer(void* instance)
{
	B3DDelete(static_cast<RendererFactory*>(instance));
}

#if !B3D_MONOLITHIC_BUILD
/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return NullRendererFactory::SystemName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfNullRenderer();
}

/**	Exit point from the plugin. Called by the engine when the plugin is unloaded. */
extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(NullRendererFactory* instance)
{
	UnloadPlugin_bsfNullRenderer(instance);
}
#endif
