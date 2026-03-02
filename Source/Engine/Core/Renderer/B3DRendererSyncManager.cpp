//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererSyncManager.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DGameObjectCollection.h"

using namespace b3d;

RendererSyncManager::RendererSyncManager()
{
	for(u32 allocatorIndex = 0; allocatorIndex < B3DSize(mSyncAllocators); allocatorIndex++)
		mSyncAllocators[allocatorIndex] = B3DNew<FrameAllocator>();
}

RendererSyncManager::~RendererSyncManager()
{
	for(u32 allocatorIndex = 0; allocatorIndex < B3DSize(mSyncAllocators); allocatorIndex++)
		B3DDelete(mSyncAllocators[allocatorIndex]);
}

void RendererSyncManager::SyncToRenderThread(bool swapBuffers)
{
	// Can happen when this is called during shutdown
	if(!SceneManager::IsStarted())
		return;

	FrameAllocator* allocator = mSyncAllocators[mActiveFrameAllocatorIndex];

	for(auto& [key, weakScene] : SceneManager::Instance().GetAllScenes())
	{
		SPtr<SceneInstance> scene = weakScene.lock();
		if(!scene)
			continue;

		ecs::Registry& registry = scene->GetGameObjectCollection()->GetECSRegistry();
		scene->GetRendererScene()->SyncToRenderThread(registry, *allocator);
	}

	if(swapBuffers)
	{
		mActiveFrameAllocatorIndex = (mActiveFrameAllocatorIndex + 1) % B3DSize(mSyncAllocators);
		mSyncAllocators[mActiveFrameAllocatorIndex]->Clear();
	}
}
