//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullRendererPrerequisites.h"
#include "BsNullRenderer.h"
#include "Renderer/BsRendererManager.h"

using namespace bs;

/**	Returns a name of the plugin. */
extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
{
	return NullRendererFactory::SystemName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
{
	RendererManager::Instance().RegisterFactoryInternal(bs_shared_ptr_new<NullRendererFactory>());

	return nullptr;
}
