//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererExtension.h"
#include "CoreThread/BsCoreThread.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"

namespace bs
{
	void RendererExtension::_initializer(RendererExtension* obj, const Any& data)
	{
		auto coreInitializer = [=]()
		{
			RendererManager::instance().GetActive()->AddPlugin(obj);
			obj->Initialize(data);
		};

		gCoreThread().QueueCommand(coreInitializer);
	}

	void RendererExtension::_deleter(RendererExtension* obj)
	{
		auto deleteObj = [=]()
		{
			RendererManager::instance().GetActive()->RemovePlugin(obj);

			obj->Destroy();
			obj->~RendererExtension();

			bs_free(obj);
		};

		// Queue deletion on the core thread
		gCoreThread().QueueCommand(deleteObj);
	}
}
