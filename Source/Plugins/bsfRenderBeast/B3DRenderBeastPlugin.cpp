//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeastPrerequisites.h"
#include "B3DRenderBeastFactory.h"

using namespace b3d;

extern "C" void* LoadPlugin_bsfRenderBeast()
{
	return static_cast<void*>(B3DNew<RenderBeastFactory>());
}

extern "C" void UnloadPlugin_bsfRenderBeast(void* instance)
{
	B3DDelete(static_cast<RendererFactory*>(instance));
}

/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return RenderBeastFactory::kSystemName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfRenderBeast();
}

/**	Exit point from the plugin. Called by the engine when the plugin is unloaded. */
extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(RenderBeastFactory* instance)
{
	UnloadPlugin_bsfRenderBeast(instance);
}
