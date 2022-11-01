//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreObjectManager.h"
#include "CoreThread/BsCoreObject.h"
#include "CoreThread/BsCoreObjectCore.h"
#include "Error/BsException.h"
#include "Math/BsMath.h"
#include "CoreThread/BsCoreThread.h"

using namespace bs;

CoreObjectManager::CoreObjectManager()
	: mNextAvailableID(1)
{
}

CoreObjectManager::~CoreObjectManager()
{
#if BS_DEBUG_MODE
	Lock lock(mObjectsMutex);

	if(mObjects.size() > 0)
	{
		// All objects MUST be destroyed at this point, otherwise there might be memory corruption.
		// (Reason: This is called on application shutdown and at that point we also unload any dynamic libraries,
		// which will invalidate any pointers to objects created from those libraries. Therefore we require of the user to
		// clean up all objects manually before shutting down the application).
		BS_EXCEPT(InternalErrorException, "Core object manager shut down, but not all objects were released. Application must release ALL "
										  "engine objects before shutdown.");
	}
#endif
}

u64 CoreObjectManager::GenerateId()
{
	Lock lock(mObjectsMutex);

	return mNextAvailableID++;
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
	assert(object != nullptr && !object->IsDestroyed());

	u64 internalId = object->GetInternalId();

	// If dirty, we generate sync data before it is destroyed
	{
		Lock lock(mObjectsMutex);
		bool isDirty = object->IsCoreDirty() || (mDirtyObjects.find(internalId) != mDirtyObjects.end());

		if(isDirty)
		{
			SPtr<ct::CoreObject> coreObject = object->GetCore();
			if(coreObject != nullptr)
			{
				CoreSyncData objSyncData = object->SyncToCore(GetCoreThread().GetFrameAlloc());

				mDestroyedSyncData.push_back(CoreStoredSyncObjData(coreObject, internalId, objSyncData));

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

		auto iterFind = mDependants.find(internalId);
		if(iterFind != mDependants.end())
		{
			Vector<CoreObject*>& dependants = iterFind->second;
			for(auto& entry : dependants)
			{
				auto iterFind2 = mDependencies.find(entry->GetInternalId());
				if(iterFind2 != mDependencies.end())
				{
					Vector<CoreObject*>& dependencies = iterFind2->second;
					auto iterFind3 = std::find(dependencies.begin(), dependencies.end(), object);

					if(iterFind3 != dependencies.end())
						dependencies.erase(iterFind3);

					if(dependencies.size() == 0)
						mDependencies.erase(iterFind2);
				}
			}

			mDependants.erase(iterFind);
		}

		mDependencies.erase(internalId);
	}
}

void CoreObjectManager::NotifyCoreDirty(CoreObject* object)
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

			auto iterFind = mDependencies.find(id);
			if(iterFind != mDependencies.end())
			{
				const Vector<CoreObject*>& oldDependencies = iterFind->second;

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
					auto iterFind2 = mDependants.find(dependencyId);

					if(iterFind2 != mDependants.end())
					{
						Vector<CoreObject*>& dependants = iterFind2->second;
						auto findIter3 = std::find(dependants.begin(), dependants.end(), object);
						dependants.erase(findIter3);

						if(dependants.size() == 0)
							mDependants.erase(iterFind2);
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

void CoreObjectManager::SyncToCore()
{
	SyncDownload(GetCoreThread().GetFrameAlloc());
	GetCoreThread().QueueCommand(std::bind(&CoreObjectManager::SyncUpload, this));
}

void CoreObjectManager::SyncToCore(CoreObject* object)
{
	struct IndividualCoreSyncData
	{
		SPtr<ct::CoreObject> Destination;
		CoreSyncData SyncData;
		FrameAlloc* Allocator;
	};

	Lock lock(mObjectsMutex);

	FrameAlloc* allocator = GetCoreThread().GetFrameAlloc();
	Vector<IndividualCoreSyncData> syncData;

	std::function<void(CoreObject*)> syncObject = [&](CoreObject* curObj)
	{
		if(!curObj->IsCoreDirty())
			return; // We already processed it as some other object's dependency

		// Sync dependencies before dependants
		// Note: I don't check for recursion. Possible infinite loop if two objects
		// are dependent on one another.

		u64 id = curObj->GetInternalId();
		auto iterFind = mDependencies.find(id);

		if(iterFind != mDependencies.end())
		{
			const Vector<CoreObject*>& dependencies = iterFind->second;
			for(auto& dependency : dependencies)
				syncObject(dependency);
		}

		SPtr<ct::CoreObject> objectCore = curObj->GetCore();
		if(objectCore == nullptr)
		{
			curObj->MarkCoreClean();
			mDirtyObjects.erase(id);
			return;
		}

		syncData.push_back(IndividualCoreSyncData());
		IndividualCoreSyncData& data = syncData.back();
		data.Allocator = allocator;
		data.Destination = objectCore;
		data.SyncData = curObj->SyncToCore(allocator);

		curObj->MarkCoreClean();
		mDirtyObjects.erase(id);
	};

	syncObject(object);

	std::function<void(const Vector<IndividualCoreSyncData>&)> callback =
		[](const Vector<IndividualCoreSyncData>& data)
	{
		// Traverse in reverse to sync dependencies before dependants
		for(auto riter = data.rbegin(); riter != data.rend(); ++riter)
		{
			const IndividualCoreSyncData& entry = *riter;
			entry.Destination->SyncToCore(entry.SyncData);

			u8* dataPtr = entry.SyncData.GetBuffer();

			if(dataPtr != nullptr)
				entry.Allocator->Free(dataPtr);
		}
	};

	if(syncData.size() > 0)
		GetCoreThread().QueueCommand(std::bind(callback, syncData));
}

void CoreObjectManager::SyncDownload(FrameAlloc* allocator)
{
	Lock lock(mObjectsMutex);

	mCoreSyncData.push_back(CoreStoredSyncData());
	CoreStoredSyncData& syncData = mCoreSyncData.back();

	syncData.Alloc = allocator;

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
					const bool wasDirty = dependant->IsCoreDirty();

					// Let the dependant objects know their dependency changed
					CoreObject* dependency = objectData.second.Object;
					dependant->OnDependencyDirty(dependency, dependency->GetCoreDirtyFlags());

					if(!wasDirty && dependant->IsCoreDirty())
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
		std::function<void(CoreObject*)> syncObject = [&](CoreObject* curObj)
		{
			if(!curObj->IsCoreDirty())
				return; // We already processed it as some other object's dependency

			// Sync dependencies before dependants
			// Note: I don't check for recursion. Possible infinite loop if two objects
			// are dependent on one another.

			u64 id = curObj->GetInternalId();
			auto iterFind = mDependencies.find(id);

			if(iterFind != mDependencies.end())
			{
				const Vector<CoreObject*>& dependencies = iterFind->second;
				for(auto& dependency : dependencies)
					syncObject(dependency);
			}

			SPtr<ct::CoreObject> objectCore = curObj->GetCore();
			if(objectCore == nullptr)
			{
				curObj->MarkCoreClean();
				return;
			}

			CoreSyncData objSyncData = curObj->SyncToCore(allocator);
			curObj->MarkCoreClean();

			syncData.Entries.push_back(CoreStoredSyncObjData(objectCore, curObj->GetInternalId(), objSyncData));
		};

		CoreObject* object = objectData.second.Object;
		if(object != nullptr)
			syncObject(object);
		else
		{
			// Object was destroyed but we still need to sync its modifications before it was destroyed
			if(objectData.second.SyncDataId != -1)
			{
				const CoreStoredSyncObjData& objData = mDestroyedSyncData[objectData.second.SyncDataId];

				syncData.Entries.push_back(objData);
				syncData.DestroyedObjects.push_back(objData.DestinationObj);
			}
		}
	}

	mDirtyObjects.clear();
	mDestroyedSyncData.clear();
}

void CoreObjectManager::SyncUpload()
{
	Lock lock(mObjectsMutex);

	if(mCoreSyncData.size() == 0)
		return;

	CoreStoredSyncData& syncData = mCoreSyncData.front();

	for(auto& objSyncData : syncData.Entries)
	{
		SPtr<ct::CoreObject> destinationObj = objSyncData.DestinationObj;
		if(destinationObj != nullptr)
			destinationObj->SyncToCore(objSyncData.SyncData);

		u8* data = objSyncData.SyncData.GetBuffer();

		if(data != nullptr)
			syncData.Alloc->Free(data);
	}

	syncData.DestroyedObjects.clear();
	syncData.Entries.clear();
	mCoreSyncData.pop_front();
}
