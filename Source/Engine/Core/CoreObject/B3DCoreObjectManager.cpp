//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreObject/B3DCoreObjectManager.h"
#include "CoreObject/B3DCoreObject.h"
#include "CoreObject/B3DRenderProxy.h"
#include "Renderer/B3DRendererSyncManager.h"
#include "CoreObject/B3DRenderThread.h"
#include "CoreObject/B3DCoreObjectSync.h"

using namespace b3d;

std::atomic<u64> CoreObjectManager::NextAvailableId = { 1 };

CoreObjectManager::CoreObjectManager()
{
	for(u32 allocatorIndex = 0; allocatorIndex < B3DSize(mSyncAllocators); allocatorIndex++)
		mSyncAllocators[allocatorIndex] = B3DNew<FrameAllocator>();

	RendererSyncManager::StartUp();
}

CoreObjectManager::~CoreObjectManager()
{
	RendererSyncManager::ShutDown();

#if B3D_DEBUG
	Lock lock(mObjectsMutex);

	if(mObjects.size() > 0)
	{
		// All objects MUST be destroyed at this point, otherwise there might be memory corruption.
		// (Reason: This is called on application shutdown and at that point we also unload any dynamic libraries,
		// which will invalidate any pointers to objects created from those libraries. Therefore we require of the user to
		// clean up all objects manually before shutting down the application).
		B3D_ASSERT(false && "Core object manager shut down, but not all objects were released. Application must release all engine objects before shutdown.");
	}
#endif

	for(u32 allocatorIndex = 0; allocatorIndex < B3DSize(mSyncAllocators); allocatorIndex++)
	{
		B3DDelete(mSyncAllocators[allocatorIndex]);
	}
}

u64 CoreObjectManager::GenerateId()
{
	return NextAvailableId.fetch_add(1);
}

void CoreObjectManager::RegisterObject(CoreObject* object)
{
	Lock lock(mObjectsMutex);

	u64 objId = object->GetInternalId();
	mObjects[objId] = object;
	mDirtyObjects[objId] = { object, -1 };
}

void CoreObjectManager::UnregisterObject(CoreObject* object)
{
	B3D_ASSERT(object != nullptr && !object->IsDestroyed());

	u64 internalId = object->GetInternalId();

	// If dirty, we generate sync data before it is destroyed
	{
		Lock lock(mObjectsMutex);
		bool isDirty = object->IsRenderProxyDataOutOfDate() || (mDirtyObjects.find(internalId) != mDirtyObjects.end());

		if(isDirty)
		{
			SPtr<render::RenderProxy> renderProxy = object->GetRenderProxy();
			if(renderProxy != nullptr)
			{
				FrameAllocator* allocator = mSyncAllocators[mActiveFrameAllocatorIndex];
				CoreSyncData objSyncData;

				RenderProxySyncPacket* const syncPacket = object->CreateRenderProxySyncPacket(*allocator, object->GetRenderProxyDirtyFlags());
				if(syncPacket != nullptr)
					objSyncData = CoreSyncData(syncPacket);

				mDestroyedSyncData.push_back(PerObjectSyncData(renderProxy, internalId, objSyncData));

				DirtyObjectData& dirtyObjData = mDirtyObjects[internalId];
				dirtyObjData.SyncDataId = (i32)mDestroyedSyncData.size() - 1;
				dirtyObjData.Object = nullptr;
			}
			else
			{
				DirtyObjectData& dirtyObjData = mDirtyObjects[internalId];
				dirtyObjData.SyncDataId = -1;
				dirtyObjData.Object = nullptr;
			}
		}

		mObjects.erase(internalId);
	}

	UpdateDependencies(object, nullptr);

	// Clear dependencies from dependants
	{
		Lock lock(mObjectsMutex);

		auto foundDependants = mDependants.find(internalId);
		if(foundDependants != mDependants.end())
		{
			Vector<CoreObject*>& dependants = foundDependants->second;
			for(auto& entry : dependants)
			{
				auto foundDependencies = mDependencies.find(entry->GetInternalId());
				if(foundDependencies != mDependencies.end())
				{
					Vector<CoreObject*>& dependencies = foundDependencies->second;
					auto foundObject = std::find(dependencies.begin(), dependencies.end(), object);

					if(foundObject != dependencies.end())
						dependencies.erase(foundObject);

					if(dependencies.size() == 0)
						mDependencies.erase(foundDependencies);
				}
			}

			mDependants.erase(foundDependants);
		}

		mDependencies.erase(internalId);
	}
}

void CoreObjectManager::NotifyRenderProxyDirty(CoreObject* object)
{
	u64 id = object->GetInternalId();

	Lock lock(mObjectsMutex);

	mDirtyObjects[id] = { object, -1 };
}

void CoreObjectManager::NotifyDependenciesDirty(CoreObject* object)
{
	Vector<CoreObject*> dependencies;
	object->GetCoreDependencies(dependencies);

	UpdateDependencies(object, &dependencies);
}

void CoreObjectManager::UpdateDependencies(CoreObject* object, Vector<CoreObject*>* dependencies)
{
	u64 id = object->GetInternalId();

	B3DMarkAllocatorFrame();
	{
		FrameVector<CoreObject*> toRemove;
		FrameVector<CoreObject*> toAdd;

		Lock lock(mObjectsMutex);

		// Add dependencies and clear old dependencies from dependants
		{
			if(dependencies != nullptr)
				std::sort(dependencies->begin(), dependencies->end());

			auto foundDependencies = mDependencies.find(id);
			if(foundDependencies != mDependencies.end())
			{
				const Vector<CoreObject*>& oldDependencies = foundDependencies->second;

				if(dependencies != nullptr)
				{
					std::set_difference(oldDependencies.begin(), oldDependencies.end(), dependencies->begin(), dependencies->end(), std::inserter(toRemove, toRemove.begin()));

					std::set_difference(dependencies->begin(), dependencies->end(), oldDependencies.begin(), oldDependencies.end(), std::inserter(toAdd, toAdd.begin()));
				}
				else
				{
					for(auto& dependency : oldDependencies)
						toRemove.push_back(dependency);
				}

				for(auto& dependency : toRemove)
				{
					u64 dependencyId = dependency->GetInternalId();
					auto foundDependants = mDependants.find(dependencyId);

					if(foundDependants != mDependants.end())
					{
						Vector<CoreObject*>& dependants = foundDependants->second;
						auto foundObject = std::find(dependants.begin(), dependants.end(), object);
						dependants.erase(foundObject);

						if(dependants.size() == 0)
							mDependants.erase(foundDependants);
					}
				}

				if(dependencies != nullptr && dependencies->size() > 0)
					mDependencies[id] = *dependencies;
				else
					mDependencies.erase(id);
			}
			else
			{
				if(dependencies != nullptr && dependencies->size() > 0)
				{
					for(auto& dependency : *dependencies)
						toAdd.push_back(dependency);

					mDependencies[id] = *dependencies;
				}
			}
		}

		// Register dependants
		{
			for(auto& dependency : toAdd)
			{
				u64 dependencyId = dependency->GetInternalId();
				Vector<CoreObject*>& dependants = mDependants[dependencyId];
				dependants.push_back(object);
			}
		}
	}
	B3DClearAllocatorFrame();
}

void CoreObjectManager::SyncToRenderThread(bool swapBuffers)
{
	Lock lock(mObjectsMutex);

	SyncDownload(mSyncAllocators[mActiveFrameAllocatorIndex]);

	auto fnSyncUpload = [this]
	{
		SyncUpload();
	};

	GetRenderThread().PostCommand(fnSyncUpload, "SyncToRenderThread");

	if(swapBuffers)
	{
		mActiveFrameAllocatorIndex = (mActiveFrameAllocatorIndex + 1) % B3DSize(mSyncAllocators);
		mSyncAllocators[mActiveFrameAllocatorIndex]->Clear();
	}

	// Note: ECS synchronization does not handle dependencies with CoreObject synchronization, therefore it is important all dependencies are
	// still CoreObject synchronized, which runs before above. This way dependencies can mark their dependants as dirty.
	RendererSyncManager::Instance().SyncToRenderThread(swapBuffers);
}

void CoreObjectManager::SyncToRenderThread(CoreObject* object)
{
	struct IndividualCoreSyncData
	{
		SPtr<render::RenderProxy> Destination;
		CoreSyncData SyncData;
		FrameAllocator* Allocator;
	};

	Lock lock(mObjectsMutex);

	FrameAllocator* allocator = mSyncAllocators[mActiveFrameAllocatorIndex];
	Vector<IndividualCoreSyncData> syncData;

	std::function<void(CoreObject*)> fnSyncObject = [&](CoreObject* currentObject)
	{
		if(!currentObject->IsRenderProxyDataOutOfDate())
			return; // We already processed it as some other object's dependency

		// Sync dependencies before dependants
		// Note: I don't check for recursion. Possible infinite loop if two objects
		// are dependent on one another.

		u64 objectId = currentObject->GetInternalId();
		auto foundDependencies = mDependencies.find(objectId);

		if(foundDependencies != mDependencies.end())
		{
			const Vector<CoreObject*>& dependencies = foundDependencies->second;
			for(auto& dependency : dependencies)
				fnSyncObject(dependency);
		}

		SPtr<render::RenderProxy> renderProxy = currentObject->GetRenderProxy();
		if(renderProxy == nullptr)
		{
			currentObject->MarkRenderProxyDataUpToDate();
			mDirtyObjects.erase(objectId);
			return;
		}

		syncData.push_back(IndividualCoreSyncData());
		IndividualCoreSyncData& data = syncData.back();
		data.Allocator = allocator;
		data.Destination = renderProxy;

		RenderProxySyncPacket* const syncPacket = currentObject->CreateRenderProxySyncPacket(*allocator, currentObject->GetRenderProxyDirtyFlags());
		if(syncPacket != nullptr)
			data.SyncData = CoreSyncData(syncPacket);

		currentObject->MarkRenderProxyDataUpToDate();
		mDirtyObjects.erase(objectId);
	};

	fnSyncObject(object);

	auto fnCallback = [](const Vector<IndividualCoreSyncData>& data)
	{
		// Traverse in reverse to sync dependencies before dependants
		for(auto reverseIterator = data.rbegin(); reverseIterator != data.rend(); ++reverseIterator)
		{
			const IndividualCoreSyncData& entry = *reverseIterator;
			entry.Destination->SyncFromCoreObject(entry.SyncData, *entry.Allocator);

			RenderProxySyncPacket* const syncPacket = entry.SyncData.GetSyncPacket();
			if(syncPacket != nullptr)
				entry.Allocator->Destruct(syncPacket);
		}
	};

	if(syncData.size() > 0)
		GetRenderThread().PostCommand([fnCallback, syncData] { fnCallback(syncData); }, "SyncToRenderThread(CoreObject*)");
}

void CoreObjectManager::SyncDownload(FrameAllocator* allocator)
{
	PerFrameSyncData syncData;
	syncData.Allocator = allocator;

	// Add all objects dependant on the dirty objects
	B3DMarkAllocatorFrame();
	{
		FrameSet<CoreObject*> dirtyDependants;
		for(auto& objectData : mDirtyObjects)
		{
			auto iterFind = mDependants.find(objectData.first);
			if(iterFind != mDependants.end())
			{
				const Vector<CoreObject*>& dependants = iterFind->second;
				for(auto& dependant : dependants)
				{
					const bool wasOutOfDate = dependant->IsRenderProxyDataOutOfDate();

					// Let the dependant objects know their dependency changed
					CoreObject* dependency = objectData.second.Object;
					dependant->OnDependencyDirty(dependency, dependency->GetRenderProxyDirtyFlags());

					if(!wasOutOfDate && dependant->IsRenderProxyDataOutOfDate())
						dirtyDependants.insert(dependant);
				}
			}
		}

		for(auto& dirtyDependant : dirtyDependants)
		{
			u64 id = dirtyDependant->GetInternalId();

			mDirtyObjects[id] = { dirtyDependant, -1 };
		}
	}

	B3DClearAllocatorFrame();

	// Order in which objects are recursed in matters, ones with lower ID will have been created before
	// ones with higher ones and should be updated first.
	for(auto& objectData : mDirtyObjects)
	{
		std::function<void(CoreObject*)> fnSyncObject = [&](CoreObject* currentObject)
		{
			if(!currentObject->IsRenderProxyDataOutOfDate())
				return; // We already processed it as some other object's dependency

			// Sync dependencies before dependants
			// Note: I don't check for recursion. Possible infinite loop if two objects
			// are dependent on one another.

			u64 objectId = currentObject->GetInternalId();
			auto foundDependencies = mDependencies.find(objectId);

			if(foundDependencies != mDependencies.end())
			{
				const Vector<CoreObject*>& dependencies = foundDependencies->second;
				for(auto& dependency : dependencies)
					fnSyncObject(dependency);
			}

			SPtr<render::RenderProxy> renderProxy = currentObject->GetRenderProxy();
			if(renderProxy == nullptr)
			{
				currentObject->MarkRenderProxyDataUpToDate();
				return;
			}

			CoreSyncData objectSyncData;

			RenderProxySyncPacket* const syncPacket = currentObject->CreateRenderProxySyncPacket(*allocator, currentObject->GetRenderProxyDirtyFlags());
			if(syncPacket != nullptr)
				objectSyncData = CoreSyncData(syncPacket);

			currentObject->MarkRenderProxyDataUpToDate();
			syncData.Entries.push_back(PerObjectSyncData(renderProxy, currentObject->GetInternalId(), objectSyncData));
		};

		CoreObject* object = objectData.second.Object;
		if(object != nullptr)
			fnSyncObject(object);
		else
		{
			// Object was destroyed but we still need to sync its modifications before it was destroyed
			if(objectData.second.SyncDataId != -1)
			{
				const PerObjectSyncData& perObjectData = mDestroyedSyncData[objectData.second.SyncDataId];

				syncData.Entries.push_back(perObjectData);
				syncData.DestroyedObjects.push_back(perObjectData.RenderProxy);
			}
		}
	}

	mDirtyObjects.clear();
	mDestroyedSyncData.clear();

	mPerFrameSyncData.emplace_back(std::move(syncData));
}

void CoreObjectManager::SyncUpload()
{
	PerFrameSyncData syncData;
	{
		Lock lock(mObjectsMutex);

		if(mPerFrameSyncData.empty())
			return;

		syncData = std::move(mPerFrameSyncData.front());
		mPerFrameSyncData.pop_front();
	}

	for(auto& objectSyncData : syncData.Entries)
	{
		SPtr<render::RenderProxy> destinationObject = objectSyncData.RenderProxy;
		if(destinationObject != nullptr)
			destinationObject->SyncFromCoreObject(objectSyncData.SyncData, *syncData.Allocator);

		RenderProxySyncPacket* const syncPacket = objectSyncData.SyncData.GetSyncPacket();
		if(syncPacket != nullptr)
			syncData.Allocator->Destruct(syncPacket);
	}

	syncData.DestroyedObjects.clear();
	syncData.Entries.clear();
}
