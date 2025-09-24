//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeastPrerequisites.h"
#include "BsRenderBeastFactory.h"
#include "Renderer/BsRendererManager.h"

using namespace b3d;

/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return RenderBeastFactory::kSystemName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	RendererManager::Instance().RegisterFactoryInternal(B3DMakeShared<RenderBeastFactory>());

	return nullptr;
}
