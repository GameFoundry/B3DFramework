//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsGameObjectManager.h"

#include "BsGameObjectCollection.h"
#include "Scene/BsGameObject.h"

using namespace bs;

GameObjectManager::~GameObjectManager()
{
	DestroyQueuedObjects();
}

GameObjectHandleBase GameObjectManager::GetObject(u64 id) const
{
	Lock lock(mMutex);

	const auto iterFind = mObjects.find(id);
	if(iterFind != mObjects.end())
		return iterFind->second;

	return nullptr;
}

bool GameObjectManager::TryGetObject(u64 id, GameObjectHandleBase& object) const
{
	Lock lock(mMutex);

	const auto iterFind = mObjects.find(id);
	if(iterFind != mObjects.end())
	{
		object = iterFind->second;
		return true;
	}

	return false;
}

bool GameObjectManager::ObjectExists(u64 id) const
{
	Lock lock(mMutex);

	return mObjects.find(id) != mObjects.end();
}

void GameObjectManager::RemapId(u64 oldId, u64 newId)
{
	if(oldId == newId)
		return;

	Lock lock(mMutex);
	mObjects[newId] = mObjects[oldId];
	mObjects.erase(oldId);
}

u64 GameObjectManager::ReserveId()
{
	return mNextAvailableID.fetch_add(1, std::memory_order_relaxed);
}

void GameObjectManager::QueueForDestroy(const GameObjectHandleBase& object)
{
	if(object.IsDestroyed())
		return;

	const u64 instanceId = object->GetInstanceId();
	mQueuedForDestroy[instanceId] = object;
}

void GameObjectManager::DestroyQueuedObjects()
{
	for(auto& objPair : mQueuedForDestroy)
		objPair.second->DestroyInternal(objPair.second, true);

	mQueuedForDestroy.clear();

	for(const auto& entry : mGameObjectCollections)
	{
		const SPtr<GameObjectCollection>& collection = entry.second.lock();
		if(!B3D_ENSURE(collection != nullptr))
			continue;

		collection->DestroyQueuedObjects();
	}
}

void GameObjectManager::RegisterGameObjectCollection(const SPtr<GameObjectCollection>& collection)
{
	if(!B3D_ENSURE(collection != nullptr))
		return;

	const UUID& id = collection->GetId();

	auto result = mGameObjectCollections.insert(std::make_pair(id, collection));
	B3D_ENSURE(result.second);
}

void GameObjectManager::UnregisterGameObjectCollection(const GameObjectCollection& collection)
{
	const UUID& id = collection.GetId();
	mGameObjectCollections.erase(id);
}

GameObjectHandleBase GameObjectManager::RegisterObject(const SPtr<GameObject>& object)
{
	const u64 id = mNextAvailableID.fetch_add(1, std::memory_order_relaxed);
	object->Initialize(object, id);

	GameObjectHandleBase handle(object);
	{
		Lock lock(mMutex);
		mObjects[id] = handle;
	}

	return handle;
}

void GameObjectManager::UnregisterObject(GameObjectHandleBase& object)
{
	{
		Lock lock(mMutex);
		mObjects.erase(object->GetInstanceId());
	}

	OnDestroyed(B3DStaticGameObjectCast<GameObject>(object));
	object.Destroy();
}

GameObjectDeserializationState::GameObjectDeserializationState(u32 options)
	: mOptions(options)
{}

GameObjectDeserializationState::~GameObjectDeserializationState()
{
	B3D_ASSERT(mUnresolvedHandles.empty() && "Deserialization state being destroyed before all handles are resolved.");
	B3D_ASSERT(mDeserializedObjects.empty() && "Deserialization state being destroyed before all objects are resolved.");
}

void GameObjectDeserializationState::Resolve()
{
	for(auto& entry : mUnresolvedHandles)
	{
		u64 instanceId = entry.OriginalInstanceId;

		bool isInternalReference = false;

		const auto findIter = mIdMapping.find(instanceId);
		if(findIter != mIdMapping.end())
		{
			if((mOptions & GODM_UseNewIds) != 0)
				instanceId = findIter->second;

			isInternalReference = true;
		}

		if(isInternalReference)
		{
			const auto findIterObj = mDeserializedObjects.find(instanceId);

			if(findIterObj != mDeserializedObjects.end())
				entry.Handle.SetObject(findIterObj->second);
			else
			{
				if((mOptions & GODM_KeepMissing) == 0)
					entry.Handle.ClearObject();
			}
		}
		else if(!isInternalReference && (mOptions & GODM_RestoreExternal) != 0)
		{
			HGameObject obj;
			if(GameObjectManager::Instance().TryGetObject(instanceId, obj))
				entry.Handle.SetObject(obj);
			else
			{
				if((mOptions & GODM_KeepMissing) == 0)
					entry.Handle.ClearObject();
			}
		}
		else
		{
			if((mOptions & GODM_KeepMissing) == 0)
				entry.Handle.ClearObject();
		}
	}

	for(auto iter = mEndCallbacks.rbegin(); iter != mEndCallbacks.rend(); ++iter)
	{
		(*iter)();
	}

	mIdMapping.clear();
	mUnresolvedHandles.clear();
	mEndCallbacks.clear();
	mUnresolvedHandleData.clear();
	mDeserializedObjects.clear();
}

void GameObjectDeserializationState::RegisterUnresolvedHandle(u64 originalId, GameObjectHandleBase& object)
{
	// All handles that are deserialized during a single begin/endDeserialization session pointing to the same object
	// must share the same GameObjectHandleData as that makes certain operations in other systems much simpler.
	// Therefore we store all the unresolved handles, and if a handle pointing to the same object was already
	// processed, or that object was already created we replace the handle's internal GameObjectHandleData.

	// Update the provided handle to ensure all handles pointing to the same object share the same handle data
	bool foundHandleData = false;

	// Search object that are currently being deserialized
	const auto iterFind = mIdMapping.find(originalId);
	if(iterFind != mIdMapping.end())
	{
		const auto iterFind2 = mDeserializedObjects.find(iterFind->second);
		if(iterFind2 != mDeserializedObjects.end())
		{
			object.mSharedHandleData = iterFind2->second.mSharedHandleData;
			foundHandleData = true;
		}
	}

	// Search previously deserialized handles
	if(!foundHandleData)
	{
		auto iterFind = mUnresolvedHandleData.find(originalId);
		if(iterFind != mUnresolvedHandleData.end())
		{
			object.mSharedHandleData = iterFind->second;
			foundHandleData = true;
		}
	}

	// If still not found, this is the first such handle so register its handle data
	if(!foundHandleData)
		mUnresolvedHandleData[originalId] = object.mSharedHandleData;

	mUnresolvedHandles.push_back({ originalId, object });
}

void GameObjectDeserializationState::RegisterObject(u64 originalId, GameObjectHandleBase& object)
{
	B3D_ASSERT(originalId != 0 && "Invalid game object ID.");

	const auto iterFind = mUnresolvedHandleData.find(originalId);
	if(iterFind != mUnresolvedHandleData.end())
	{
		SPtr<GameObject> ptr = object.GetShared();

		object.mSharedHandleData = iterFind->second;
		object.SetObject(ptr);
	}

	const u64 newId = object->GetInstanceId();
	mIdMapping[originalId] = newId;
	mDeserializedObjects[newId] = object;
}

void GameObjectDeserializationState::RegisterOnDeserializationEndCallback(std::function<void()> callback)
{
	mEndCallbacks.push_back(callback);
}
