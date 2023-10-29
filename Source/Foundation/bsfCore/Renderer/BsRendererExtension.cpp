//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererExtension.h"
#include "CoreObject/BsRenderThread.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"

using namespace bs;

void RendererExtension::InitializerInternal(RendererExtension* obj, const Any& data)
{
	auto coreInitializer = [=]()
	{
		RendererManager::Instance().GetActive()->AddPlugin(obj);
		obj->Initialize(data);
	};

	GetRenderThread().PostCommand(coreInitializer);
}

void RendererExtension::DeleterInternal(RendererExtension* obj)
{
	auto deleteObj = [=]()
	{
		RendererManager::Instance().GetActive()->RemovePlugin(obj);

		obj->Destroy();
		obj->~RendererExtension();

		B3DFree(obj);
	};

	// Queue deletion on the render thread
	GetRenderThread().PostCommand(deleteObj);
}
