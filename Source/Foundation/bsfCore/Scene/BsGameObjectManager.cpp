//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsGameObject.h"

namespace bs
{
	GameObjectManager::~GameObjectManager()
	{
		destroyQueuedObjects();
	}

	GameObjectHandleBase GameObjectManager::GetObject(UINT64 id) const
	{
		Lock Lock(mMutex);

		const auto iterFind = mObjects.Find(id);
		if (iterFind != mObjects.End())
			return iterFind->second;

		return nullptr;
	}

	bool GameObjectManager::TryGetObject(UINT64 id, GameObjectHandleBase& object) const
	{
		Lock Lock(mMutex);

		const auto iterFind = mObjects.Find(id);
		if (iterFind != mObjects.End())
		{
			object = iterFind->second;
			return true;
		}

		return false;
	}

	bool GameObjectManager::ObjectExists(UINT64 id) const
	{
		Lock Lock(mMutex);

		return mObjects.Find(id) != mObjects.end();
	}

	void GameObjectManager::RemapId(UINT64 oldId, UINT64 newId)
	{
		if (oldId == newId)
			return;

		Lock Lock(mMutex);
		mObjects[newId] = mObjects[oldId];
		mObjects.Erase(oldId);
	}

	UINT64 GameObjectManager::ReserveId()
	{
		return mNextAvailableID.fetch_add(1, std::memory_order_relaxed);
	}

	void GameObjectManager::QueueForDestroy(const GameObjectHandleBase& object)
	{
		if (object.IsDestroyed())
			return;

		const UINT64 instanceId = object->GetInstanceId();
		mQueuedForDestroy[instanceId] = object;
	}

	void GameObjectManager::DestroyQueuedObjects()
	{
		for (auto& objPair : mQueuedForDestroy)
			objPair.second->DestroyInternal(objPair.second, true);

		mQueuedForDestroy.Clear();
	}

	GameObjectHandleBase GameObjectManager::RegisterObject(const SPtr<GameObject>& object)
	{
		const UINT64 id = mNextAvailableID.fetch_add(1, std::memory_order_relaxed);
		object->Initialize(object, id);

		GameObjectHandleBase Handle(object);
		{
			Lock Lock(mMutex);
			mObjects[id] = handle;
		}

		return handle;
	}

	void GameObjectManager::UnregisterObject(GameObjectHandleBase& object)
	{
		{
			Lock Lock(mMutex);
			mObjects.Erase(object->GetInstanceId());
		}

		onDestroyed(static_object_cast<GameObject>(object));
		object.Destroy();
	}

	GameObjectDeserializationState::GameObjectDeserializationState(UINT32 options)
		:mOptions(options)
	{ }

	GameObjectDeserializationState::~GameObjectDeserializationState()
	{
		BS_ASSERT(mUnresolvedHandles.Empty() && "Deserialization state being destroyed before all handles are resolved.");
		BS_ASSERT(mDeserializedObjects.Empty() && "Deserialization state being destroyed before all objects are resolved.");
	}

	void GameObjectDeserializationState::Resolve()
	{
		for (auto& entry : mUnresolvedHandles)
		{
			UINT64 instanceId = entry.originalInstanceId;

			bool isInternalReference = false;

			const auto findIter = mIdMapping.Find(instanceId);
			if (findIter != mIdMapping.End())
			{
				if ((mOptions & GODM_UseNewIds) != 0)
					instanceId = findIter->second;

				isInternalReference = true;
			}

			if (isInternalReference)
			{
				const auto findIterObj = mDeserializedObjects.Find(instanceId);

				if (findIterObj != mDeserializedObjects.End())
					entry.handle._resolve(findIterObj->second);
				else
				{
					if ((mOptions & GODM_KeepMissing) == 0)
						entry.handle._resolve(nullptr);
				}
			}
			else if (!isInternalReference && (mOptions & GODM_RestoreExternal) != 0)
			{
				HGameObject obj;
				if(GameObjectManager::instance().TryGetObject(instanceId, obj))
					entry.handle._resolve(obj);
				else
				{
					if ((mOptions & GODM_KeepMissing) == 0)
						entry.handle._resolve(nullptr);
				}
			}
			else
			{
				if ((mOptions & GODM_KeepMissing) == 0)
					entry.handle._resolve(nullptr);
			}
		}

		for (auto iter = mEndCallbacks.Rbegin(); iter != mEndCallbacks.rend(); ++iter)
		{
			(*iter)();
		}

		mIdMapping.Clear();
		mUnresolvedHandles.Clear();
		mEndCallbacks.Clear();
		mUnresolvedHandleData.Clear();
		mDeserializedObjects.Clear();
	}

	void GameObjectDeserializationState::RegisterUnresolvedHandle(UINT64 originalId, GameObjectHandleBase& object)
	{
		// All handles that are deserialized during a single begin/endDeserialization session pointing to the same object
		// must share the same GameObjectHandleData as that makes certain operations in other systems much simpler.
		// Therefore we store all the unresolved handles, and if a handle pointing to the same object was already
		// processed, or that object was already created we replace the handle's internal GameObjectHandleData.

		// Update the provided handle to ensure all handles pointing to the same object share the same handle data
		bool foundHandleData = false;

		// Search object that are currently being deserialized
		const auto iterFind = mIdMapping.Find(originalId);
		if (iterFind != mIdMapping.End())
		{
			const auto iterFind2 = mDeserializedObjects.Find(iterFind->second);
			if (iterFind2 != mDeserializedObjects.End())
			{
				object.mData = iterFind2->second.mData;
				foundHandleData = true;
			}
		}

		// Search previously deserialized handles
		if (!foundHandleData)
		{
			auto iterFind = mUnresolvedHandleData.Find(originalId);
			if (iterFind != mUnresolvedHandleData.End())
			{
				object.mData = iterFind->second;
				foundHandleData = true;
			}
		}

		// If still not found, this is the first such handle so register its handle data
		if (!foundHandleData)
			mUnresolvedHandleData[originalId] = object.mData;

		mUnresolvedHandles.push_back({ originalId, object });
	}

	void GameObjectDeserializationState::RegisterObject(UINT64 originalId, GameObjectHandleBase& object)
	{
		assert(originalId != 0 && "Invalid game object ID.");

		const auto iterFind = mUnresolvedHandleData.Find(originalId);
		if (iterFind != mUnresolvedHandleData.End())
		{
			SPtr<GameObject> ptr = object.GetInternalPtr();

			object.mData = iterFind->second;
			object._setHandleData(ptr);
		}

		const UINT64 newId = object->GetInstanceId();
		mIdMapping[originalId] = newId;
		mDeserializedObjects[newId] = object;
	}

	void GameObjectDeserializationState::RegisterOnDeserializationEndCallback(std::function<void()> callback)
	{
		mEndCallbacks.push_back(callback);
	}
}
