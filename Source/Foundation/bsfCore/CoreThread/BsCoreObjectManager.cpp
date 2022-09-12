//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreObjectManager.h"
#include "CoreThread/BsCoreObject.h"
#include "CoreThread/BsCoreObjectCore.h"
#include "Error/BsException.h"
#include "Math/BsMath.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	CoreObjectManager::CoreObjectManager()
		:mNextAvailableID(1)
	{

	}

	CoreObjectManager::~CoreObjectManager()
	{
#if BS_DEBUG_MODE
		Lock Lock(mObjectsMutex);

		if(mObjects.Size() > 0)
		{
			// All objects MUST be destroyed at this point, otherwise there might be memory corruption.
			// (Reason: This is called on application shutdown and at that point we also unload any dynamic libraries,
			// which will invalidate any pointers to objects created from those libraries. Therefore we require of the user to
			// clean up all objects manually before shutting down the application).
			BS_EXCEPT(InternalErrorException, "Core object manager shut down, but not all objects were released. Application must release ALL " \
				"engine objects before shutdown.");
		}
#endif
	}

	UINT64 CoreObjectManager::GenerateId()
	{
		Lock Lock(mObjectsMutex);

		return mNextAvailableID++;
	}

	void CoreObjectManager::RegisterObject(CoreObject* object)
	{
		Lock Lock(mObjectsMutex);

		UINT64 objId = object->GetInternalID();
		mObjects[objId] = object;
		mDirtyObjects[objId] = { object, -1 };
	}

	void CoreObjectManager::UnregisterObject(CoreObject* object)
	{
		assert(object != nullptr && !object->IsDestroyed());

		UINT64 internalId = object->GetInternalID();

		// If dirty, we generate sync data before it is destroyed
		{
			Lock Lock(mObjectsMutex);
			bool isDirty = object->IsCoreDirty() || (mDirtyObjects.Find(internalId) != mDirtyObjects.end());

			if (isDirty)
			{
				SPtr<ct::CoreObject> coreObject = object->GetCore();
				if (coreObject != nullptr)
				{
					CoreSyncData objSyncData = object->SyncToCore(gCoreThread().GetFrameAlloc());
				
					mDestroyedSyncData.push_back(CoreStoredSyncObjData(coreObject, internalId, objSyncData));

					DirtyObjectData& dirtyObjData = mDirtyObjects[internalId];
					dirtyObjData.syncDataId = (INT32)mDestroyedSyncData.Size() - 1;
					dirtyObjData.object = nullptr;
				}
				else
				{
					DirtyObjectData& dirtyObjData = mDirtyObjects[internalId];
					dirtyObjData.syncDataId = -1;
					dirtyObjData.object = nullptr;
				}
			}

			mObjects.Erase(internalId);
		}

		updateDependencies(object, nullptr);

		// Clear dependencies from dependants
		{
			Lock Lock(mObjectsMutex);

			auto iterFind = mDependants.Find(internalId);
			if (iterFind != mDependants.End())
			{
				Vector<CoreObject*>& dependants = iterFind->second;
				for (auto& entry : dependants)
				{
					auto iterFind2 = mDependencies.Find(entry->GetInternalID());
					if (iterFind2 != mDependencies.End())
					{
						Vector<CoreObject*>& dependencies = iterFind2->second;
						auto iterFind3 = std::find(dependencies.Begin(), dependencies.end(), object);

						if (iterFind3 != dependencies.End())
							dependencies.Erase(iterFind3);

						if (dependencies.Size() == 0)
							mDependencies.Erase(iterFind2);
					}
				}

				mDependants.Erase(iterFind);
			}

			mDependencies.Erase(internalId);
		}
	}

	void CoreObjectManager::NotifyCoreDirty(CoreObject* object)
	{
		UINT64 id = object->GetInternalID();

		Lock Lock(mObjectsMutex);

		mDirtyObjects[id] = { object, -1 };
	}

	void CoreObjectManager::NotifyDependenciesDirty(CoreObject* object)
	{
		Vector<CoreObject*> dependencies;
		object->GetCoreDependencies(dependencies);

		updateDependencies(object, &dependencies);
	}

	void CoreObjectManager::UpdateDependencies(CoreObject* object, Vector<CoreObject*>* dependencies)
	{
		UINT64 id = object->GetInternalID();

		bs_frame_mark();
		{
			FrameVector<CoreObject*> toRemove;
			FrameVector<CoreObject*> toAdd;

			Lock Lock(mObjectsMutex);

			// Add dependencies and clear old dependencies from dependants
			{
				if (dependencies != nullptr)
					std::sort(dependencies->Begin(), dependencies->end());

				auto iterFind = mDependencies.Find(id);
				if (iterFind != mDependencies.End())
				{
					const Vector<CoreObject*>& oldDependencies = iterFind->second;

					if (dependencies != nullptr)
					{
						std::set_difference(oldDependencies.Begin(), oldDependencies.end(),
							dependencies->Begin(), dependencies->end(), std::inserter(toRemove, toRemove.Begin()));

						std::set_difference(dependencies->Begin(), dependencies->end(),
							oldDependencies.Begin(), oldDependencies.end(), std::inserter(toAdd, toAdd.begin()));
					}
					else
					{
						for (auto& dependency : oldDependencies)
							toRemove.push_back(dependency);
					}

					for (auto& dependency : toRemove)
					{
						UINT64 dependencyId = dependency->GetInternalID();
						auto iterFind2 = mDependants.Find(dependencyId);

						if (iterFind2 != mDependants.End())
						{
							Vector<CoreObject*>& dependants = iterFind2->second;
							auto findIter3 = std::find(dependants.Begin(), dependants.end(), object);
							dependants.Erase(findIter3);

							if (dependants.Size() == 0)
								mDependants.Erase(iterFind2);
						}
					}

					if (dependencies != nullptr && dependencies->Size() > 0)
						mDependencies[id] = *dependencies;
					else
						mDependencies.Erase(id);
				}
				else
				{
					if (dependencies != nullptr && dependencies->Size() > 0)
					{
						for (auto& dependency : *dependencies)
							toAdd.push_back(dependency);

						mDependencies[id] = *dependencies;
					}
				}
			}

			// Register dependants
			{
				for (auto& dependency : toAdd)
				{
					UINT64 dependencyId = dependency->GetInternalID();
					Vector<CoreObject*>& dependants = mDependants[dependencyId];
					dependants.push_back(object);
				}
			}
		}
		bs_frame_clear();
	}

	void CoreObjectManager::SyncToCore()
	{
		syncDownload(gCoreThread().GetFrameAlloc());
		gCoreThread().QueueCommand(std::bind(&CoreObjectManager::syncUpload, this));
	}

	void CoreObjectManager::SyncToCore(CoreObject* object)
	{
		struct IndividualCoreSyncData
		{
			SPtr<ct::CoreObject> destination;
			CoreSyncData syncData;
			FrameAlloc* allocator;
		};

		Lock Lock(mObjectsMutex);

		FrameAlloc* allocator = gCoreThread().GetFrameAlloc();
		Vector<IndividualCoreSyncData> syncData;

		std::function<void(CoreObject*)> syncObject = [&](CoreObject* curObj)
		{
			if (!curObj->IsCoreDirty())
				return; // We already processed it as some other object's dependency

			// Sync dependencies before dependants
			// Note: I don't check for recursion. Possible infinite loop if two objects
			// are dependent on one another.

			UINT64 id = curObj->GetInternalID();
			auto iterFind = mDependencies.Find(id);

			if (iterFind != mDependencies.End())
			{
				const Vector<CoreObject*>& dependencies = iterFind->second;
				for (auto& dependency : dependencies)
					syncObject(dependency);
			}

			SPtr<ct::CoreObject> objectCore = curObj->GetCore();
			if (objectCore == nullptr)
			{
				curObj->MarkCoreClean();
				mDirtyObjects.Erase(id);
				return;
			}

			syncData.push_back(IndividualCoreSyncData());
			IndividualCoreSyncData& data = syncData.Back();
			data.allocator = allocator;
			data.destination = objectCore;
			data.syncData = curObj->SyncToCore(allocator);

			curObj->MarkCoreClean();
			mDirtyObjects.Erase(id);
		};

		syncObject(object);

		std::function<void(const Vector<IndividualCoreSyncData>&)> callback =
			[](const Vector<IndividualCoreSyncData>& data)
		{
			// Traverse in reverse to sync dependencies before dependants
			for (auto riter = data.Rbegin(); riter != data.rend(); ++riter)
			{
				const IndividualCoreSyncData& entry = *riter;
				entry.destination->SyncToCore(entry.syncData);

				UINT8* dataPtr = entry.syncData.GetBuffer();

				if (dataPtr != nullptr)
					entry.allocator->Free(dataPtr);
			}
		};

		if (syncData.Size() > 0)
			gCoreThread().QueueCommand(std::bind(callback, syncData));
	}

	void CoreObjectManager::SyncDownload(FrameAlloc* allocator)
	{
		Lock Lock(mObjectsMutex);

		mCoreSyncData.push_back(CoreStoredSyncData());
		CoreStoredSyncData& syncData = mCoreSyncData.Back();

		syncData.alloc = allocator;
		
		// Add all objects dependant on the dirty objects
		bs_frame_mark();
		{
			FrameSet<CoreObject*> dirtyDependants;
			for (auto& objectData : mDirtyObjects)
			{
				auto iterFind = mDependants.Find(objectData.first);
				if (iterFind != mDependants.End())
				{
					const Vector<CoreObject*>& dependants = iterFind->second;
					for (auto& dependant : dependants)
					{
						const bool wasDirty = dependant->IsCoreDirty();

						// Let the dependant objects know their dependency changed
						CoreObject* dependency = objectData.second.object;
						dependant->OnDependencyDirty(dependency, dependency->getCoreDirtyFlags());

						if (!wasDirty && dependant->IsCoreDirty())
							dirtyDependants.Insert(dependant);
					}
				}
			}

			for (auto& dirtyDependant : dirtyDependants)
			{
				UINT64 id = dirtyDependant->GetInternalID();

				mDirtyObjects[id] = { dirtyDependant, -1 };
			}
		}

		bs_frame_clear();
		
		// Order in which objects are recursed in matters, ones with lower ID will have been created before
		// ones with higher ones and should be updated first.
		for (auto& objectData : mDirtyObjects)
		{
			std::function<void(CoreObject*)> syncObject = [&](CoreObject* curObj)
			{
				if (!curObj->IsCoreDirty())
					return; // We already processed it as some other object's dependency

				// Sync dependencies before dependants
				// Note: I don't check for recursion. Possible infinite loop if two objects
				// are dependent on one another.
				
				UINT64 id = curObj->GetInternalID();
				auto iterFind = mDependencies.Find(id);

				if (iterFind != mDependencies.End())
				{
					const Vector<CoreObject*>& dependencies = iterFind->second;
					for (auto& dependency : dependencies)
						syncObject(dependency);
				}

				SPtr<ct::CoreObject> objectCore = curObj->GetCore();
				if (objectCore == nullptr)
				{
					curObj->MarkCoreClean();
					return;
				}

				CoreSyncData objSyncData = curObj->SyncToCore(allocator);
				curObj->MarkCoreClean();

				syncData.entries.push_back(CoreStoredSyncObjData(objectCore,
					curObj->GetInternalID(), objSyncData));
			};

			CoreObject* object = objectData.second.object;
			if (object != nullptr)
				syncObject(object);
			else
			{
				// Object was destroyed but we still need to sync its modifications before it was destroyed
				if (objectData.second.syncDataId != -1)
				{
					const CoreStoredSyncObjData& objData = mDestroyedSyncData[objectData.second.syncDataId];

					syncData.entries.push_back(objData);
					syncData.destroyedObjects.push_back(objData.destinationObj);
				}
			}
		}

		mDirtyObjects.Clear();
		mDestroyedSyncData.Clear();
	}

	void CoreObjectManager::SyncUpload()
	{
		Lock Lock(mObjectsMutex);

		if (mCoreSyncData.Size() == 0)
			return;

		CoreStoredSyncData& syncData = mCoreSyncData.Front();

		for (auto& objSyncData : syncData.entries)
		{
			SPtr<ct::CoreObject> destinationObj = objSyncData.destinationObj;
			if (destinationObj != nullptr)
				destinationObj->SyncToCore(objSyncData.syncData);

			UINT8* data = objSyncData.syncData.GetBuffer();

			if (data != nullptr)
				syncData.alloc->Free(data);
		}

		syncData.destroyedObjects.Clear();
		syncData.entries.Clear();
		mCoreSyncData.pop_front();
	}
}
