//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderBeastPrerequisites.h"
#include "BsRenderBeastFactory.h"
#include "BsRendererManager.h"

namespace bs
{
	/**	Returns a name of the plugin. */
	extern "C" BS_BSRND_EXPORT const char* getPluginName()
	{
		return SystemName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_BSRND_EXPORT void* loadPlugin()
	{
		RendererManager::instance()._registerFactory(bs_shared_ptr_new<RenderBeastFactory>());

		return nullptr;
	}
}