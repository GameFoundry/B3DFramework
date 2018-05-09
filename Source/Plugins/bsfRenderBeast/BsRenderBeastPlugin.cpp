//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeastPrerequisites.h"
#include "BsRenderBeastFactory.h"
#include "Renderer/BsRendererManager.h"

namespace bs
{
	/**	Returns a name of the plugin. */
	extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
	{
		return RenderBeastFactory::SystemName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
	{
		RendererManager::instance()._registerFactory(bs_shared_ptr_new<RenderBeastFactory>());

		return nullptr;
	}
}
