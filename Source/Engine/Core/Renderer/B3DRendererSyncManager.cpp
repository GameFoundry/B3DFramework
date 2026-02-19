//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererSyncManager.h"
#include "CoreObject/B3DRenderThread.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DGameObjectCollection.h"

using namespace b3d;

namespace
{
	Vector<RendererSyncManager::HandlerFactory>& GetPendingRegistrations()
	{
		static Vector<RendererSyncManager::HandlerFactory> registrations;
		return registrations;
	}
}

RendererSyncManager::RendererSyncManager()
{
	for(u32 allocatorIndex = 0; allocatorIndex < B3DSize(mSyncAllocators); allocatorIndex++)
		mSyncAllocators[allocatorIndex] = B3DNew<FrameAllocator>();

	for(auto& pending : GetPendingRegistrations())
		mHandlerFactories.push_back(std::move(pending));

	GetPendingRegistrations().clear();
}

RendererSyncManager::~RendererSyncManager()
{
	for(u32 allocatorIndex = 0; allocatorIndex < B3DSize(mSyncAllocators); allocatorIndex++)
		B3DDelete(mSyncAllocators[allocatorIndex]);
}

void RendererSyncManager::RegisterHandlerFactoryAtLoadTime(HandlerFactory factory)
{
	GetPendingRegistrations().push_back(std::move(factory));
}

Vector<UPtr<IRendererObjectSyncHandler>> RendererSyncManager::CreateHandlers(RendererScene& rendererScene)
{
	Vector<UPtr<IRendererObjectSyncHandler>> handlers;
	for(auto& factory : mHandlerFactories)
		handlers.push_back(factory(rendererScene));

	return handlers;
}

void RendererSyncManager::SyncToRenderThread(bool swapBuffers)
{
	Lock lock(mSyncDataMutex);

	SyncRead(mSyncAllocators[mActiveFrameAllocatorIndex]);

	GetRenderThread().PostCommand([this] { SyncWrite(); }, "RendererSyncManager::SyncWrite");

	if(swapBuffers)
	{
		mActiveFrameAllocatorIndex = (mActiveFrameAllocatorIndex + 1) % B3DSize(mSyncAllocators);
		mSyncAllocators[mActiveFrameAllocatorIndex]->Clear();
	}
}

void RendererSyncManager::SyncRead(FrameAllocator* allocator)
{
	PerFrameSyncData syncData;
	syncData.Allocator = allocator;

	for(auto& [scenePtr, weakScene] : SceneManager::Instance().GetAllScenes())
	{
		SPtr<SceneInstance> scene = weakScene.lock();
		if(!scene)
			continue;

		ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();
		const auto& handlers = scene->GetRendererScene()->GetSyncHandlers();

		for(auto& handler : handlers)
		{
			void* batchData = handler->SyncRead(registry, *allocator);
			if(batchData != nullptr)
				syncData.HandlerData.Add({handler.get(), batchData});
		}
	}

	mPerFrameSyncData.emplace_back(std::move(syncData));
}

void RendererSyncManager::SyncWrite()
{
	PerFrameSyncData syncData;
	{
		Lock lock(mSyncDataMutex);

		if(mPerFrameSyncData.empty())
			return;

		syncData = std::move(mPerFrameSyncData.front());
		mPerFrameSyncData.pop_front();
	}

	for(u32 handlerIndex = 0; handlerIndex < (u32)syncData.HandlerData.Size(); ++handlerIndex)
	{
		auto& handlerData = syncData.HandlerData[handlerIndex];
		handlerData.Handler->SyncWrite(handlerData.BatchData, *syncData.Allocator);
	}
}
