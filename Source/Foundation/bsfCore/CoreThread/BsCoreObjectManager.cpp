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
		Lock lock(mObjectsMutex);

		if(mObjects.size() > 0)
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

	UINT64 CoreObjectManager::generateId()
	{
		Lock lock(mObjectsMutex);

		return mNextAvailableID++;
	}

	void CoreObjectManager::registerObject(CoreObject* object)
	{
		Lock lock(mObjectsMutex);

		UINT64 objId = object->getInternalID();
		mObjects[objId] = object;
		mDirtyObjects[objId] = { object, -1 };
	}

	void CoreObjectManager::unregisterObject(CoreObject* object)
	{
		assert(object != nullptr);

		UINT64 internalId = object->getInternalID();

		// If dirty, we generate sync data before it is destroyed
		{
			Lock lock(mObjectsMutex);
			bool isDirty = object->isCoreDirty() || (mDirtyObjects.find(internalId) != mDirtyObjects.end());

			if (isDirty)
			{
				SPtr<ct::CoreObject> coreObject = object->getCore();
				if (coreObject != nullptr)
				{
					CoreSyncData objSyncData = object->syncToCore(gCoreThread().getFrameAlloc());
				
					mDestroyedSyncData.push_back(CoreStoredSyncObjData(coreObject, internalId, objSyncData));

					DirtyObjectData& dirtyObjData = mDirtyObjects[internalId];
					dirtyObjData.syncDataId = (INT32)mDestroyedSyncData.size() - 1;
					dirtyObjData.object = nullptr;
				}
				else
				{
					DirtyObjectData& dirtyObjData = mDirtyObjects[internalId];
					dirtyObjData.syncDataId = -1;
					dirtyObjData.object = nullptr;
				}
			}

			mObjects.erase(internalId);
		}

		updateDependencies(object, nullptr);

		// Clear dependencies from dependants
		{
			Lock lock(mObjectsMutex);

			auto iterFind = mDependants.find(internalId);
			if (iterFind != mDependants.end())
			{
				Vector<CoreObject*>& dependants = iterFind->second;
				for (auto& entry : dependants)
				{
					auto iterFind2 = mDependencies.find(entry->getInternalID());
					if (iterFind2 != mDependencies.end())
					{
						Vector<CoreObject*>& dependencies = iterFind2->second;
						auto iterFind3 = std::find(dependencies.begin(), dependencies.end(), object);

						if (iterFind3 != dependencies.end())
							dependencies.erase(iterFind3);

						if (dependencies.size() == 0)
							mDependencies.erase(iterFind2);
					}
				}

				mDependants.erase(iterFind);
			}

			mDependencies.erase(internalId);
		}
	}

	void CoreObjectManager::notifyCoreDirty(CoreObject* object)
	{
		UINT64 id = object->getInternalID();

		Lock lock(mObjectsMutex);

		mDirtyObjects[id] = { object, -1 };
	}

	void CoreObjectManager::notifyDependenciesDirty(CoreObject* object)
	{
		Vector<CoreObject*> dependencies;
		object->getCoreDependencies(dependencies);

		updateDependencies(object, &dependencies);
	}

	void CoreObjectManager::updateDependencies(CoreObject* object, Vector<CoreObject*>* dependencies)
	{
		UINT64 id = object->getInternalID();

		bs_frame_mark();
		{
			FrameVector<CoreObject*> toRemove;
			FrameVector<CoreObject*> toAdd;

			Lock lock(mObjectsMutex);

			// Add dependencies and clear old dependencies from dependants
			{
				if (dependencies != nullptr)
					std::sort(dependencies->begin(), dependencies->end());

				auto iterFind = mDependencies.find(id);
				if (iterFind != mDependencies.end())
				{
					const Vector<CoreObject*>& oldDependencies = iterFind->second;

					if (dependencies != nullptr)
					{
						std::set_difference(oldDependencies.begin(), oldDependencies.end(),
							dependencies->begin(), dependencies->end(), std::inserter(toRemove, toRemove.begin()));

						std::set_difference(dependencies->begin(), dependencies->end(),
							oldDependencies.begin(), oldDependencies.end(), std::inserter(toAdd, toAdd.begin()));
					}
					else
					{
						for (auto& dependency : oldDependencies)
							toRemove.push_back(dependency);
					}

					for (auto& dependency : toRemove)
					{
						UINT64 dependencyId = dependency->getInternalID();
						auto iterFind2 = mDependants.find(dependencyId);

						if (iterFind2 != mDependants.end())
						{
							Vector<CoreObject*>& dependants = iterFind2->second;
							auto findIter3 = std::find(dependants.begin(), dependants.end(), object);
							dependants.erase(findIter3);

							if (dependants.size() == 0)
								mDependants.erase(iterFind2);
						}
					}

					if (dependencies != nullptr && dependencies->size() > 0)
						mDependencies[id] = *dependencies;
					else
						mDependencies.erase(id);
				}
				else
				{
					if (dependencies != nullptr && dependencies->size() > 0)
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
					UINT64 dependencyId = dependency->getInternalID();
					Vector<CoreObject*>& dependants = mDependants[dependencyId];
					dependants.push_back(object);
				}
			}
		}
		bs_frame_clear();
	}

	void CoreObjectManager::syncToCore()
	{
		syncDownload(gCoreThread().getFrameAlloc());
		gCoreThread().queueCommand(std::bind(&CoreObjectManager::syncUpload, this));
	}

	void CoreObjectManager::syncToCore(CoreObject* object)
	{
		struct IndividualCoreSyncData
		{
			SPtr<ct::CoreObject> destination;
			CoreSyncData syncData;
			FrameAlloc* allocator;
		};

		Lock lock(mObjectsMutex);

		FrameAlloc* allocator = gCoreThread().getFrameAlloc();
		Vector<IndividualCoreSyncData> syncData;

		std::function<void(CoreObject*)> syncObject = [&](CoreObject* curObj)
		{
			if (!curObj->isCoreDirty())
				return; // We already processed it as some other object's dependency

			// Sync dependencies before dependants
			// Note: I don't check for recursion. Possible infinite loop if two objects
			// are dependent on one another.

			UINT64 id = curObj->getInternalID();
			auto iterFind = mDependencies.find(id);

			if (iterFind != mDependencies.end())
			{
				const Vector<CoreObject*>& dependencies = iterFind->second;
				for (auto& dependency : dependencies)
					syncObject(dependency);
			}

			SPtr<ct::CoreObject> objectCore = curObj->getCore();
			if (objectCore == nullptr)
			{
				curObj->markCoreClean();
				mDirtyObjects.erase(id);
				return;
			}

			syncData.push_back(IndividualCoreSyncData());
			IndividualCoreSyncData& data = syncData.back();
			data.allocator = allocator;
			data.destination = objectCore;
			data.syncData = curObj->syncToCore(allocator);

			curObj->markCoreClean();
			mDirtyObjects.erase(id);
		};

		syncObject(object);

		std::function<void(const Vector<IndividualCoreSyncData>&)> callback =
			[](const Vector<IndividualCoreSyncData>& data)
		{
			// Traverse in reverse to sync dependencies before dependants
			for (auto riter = data.rbegin(); riter != data.rend(); ++riter)
			{
				const IndividualCoreSyncData& entry = *riter;
				entry.destination->syncToCore(entry.syncData);

				UINT8* dataPtr = entry.syncData.getBuffer();

				if (dataPtr != nullptr)
					entry.allocator->free(dataPtr);
			}
		};

		if (syncData.size() > 0)
			gCoreThread().queueCommand(std::bind(callback, syncData));
	}

	void CoreObjectManager::syncDownload(FrameAlloc* allocator)
	{
		Lock lock(mObjectsMutex);

		mCoreSyncData.push_back(CoreStoredSyncData());
		CoreStoredSyncData& syncData = mCoreSyncData.back();

		syncData.alloc = allocator;
		
		// Add all objects dependant on the dirty objects
		bs_frame_mark();
		{
			FrameSet<CoreObject*> dirtyDependants;
			for (auto& objectData : mDirtyObjects)
			{
				auto iterFind = mDependants.find(objectData.first);
				if (iterFind != mDependants.end())
				{
					const Vector<CoreObject*>& dependants = iterFind->second;
					for (auto& dependant : dependants)
					{
						const bool wasDirty = dependant->isCoreDirty();

						// Let the dependant objects know their dependency changed
						CoreObject* dependency = objectData.second.object;
						dependant->onDependencyDirty(dependency, dependency->getCoreDirtyFlags());

						if (!wasDirty && dependant->isCoreDirty())
							dirtyDependants.insert(dependant);
					}
				}
			}

			for (auto& dirtyDependant : dirtyDependants)
			{
				UINT64 id = dirtyDependant->getInternalID();

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
				if (!curObj->isCoreDirty())
					return; // We already processed it as some other object's dependency

				// Sync dependencies before dependants
				// Note: I don't check for recursion. Possible infinite loop if two objects
				// are dependent on one another.
				
				UINT64 id = curObj->getInternalID();
				auto iterFind = mDependencies.find(id);

				if (iterFind != mDependencies.end())
				{
					const Vector<CoreObject*>& dependencies = iterFind->second;
					for (auto& dependency : dependencies)
						syncObject(dependency);
				}

				SPtr<ct::CoreObject> objectCore = curObj->getCore();
				if (objectCore == nullptr)
				{
					curObj->markCoreClean();
					return;
				}

				CoreSyncData objSyncData = curObj->syncToCore(allocator);
				curObj->markCoreClean();

				syncData.entries.push_back(CoreStoredSyncObjData(objectCore,
					curObj->getInternalID(), objSyncData));
			};

			CoreObject* object = objectData.second.object;
			if (object != nullptr)
				syncObject(object);
			else
			{
				// Object was destroyed but we still need to sync its modifications before it was destroyed
				if (objectData.second.syncDataId != -1)
					syncData.entries.push_back(mDestroyedSyncData[objectData.second.syncDataId]);
			}
		}

		mDirtyObjects.clear();
		mDestroyedSyncData.clear();
	}

	void CoreObjectManager::syncUpload()
	{
		Lock lock(mObjectsMutex);

		if (mCoreSyncData.size() == 0)
			return;

		CoreStoredSyncData& syncData = mCoreSyncData.front();

		for (auto& objSyncData : syncData.entries)
		{
			SPtr<ct::CoreObject> destinationObj = objSyncData.destinationObj;
			if (destinationObj != nullptr)
				destinationObj->syncToCore(objSyncData.syncData);

			UINT8* data = objSyncData.syncData.getBuffer();

			if (data != nullptr)
				syncData.alloc->free(data);
		}

		syncData.entries.clear();
		mCoreSyncData.pop_front();
	}
}