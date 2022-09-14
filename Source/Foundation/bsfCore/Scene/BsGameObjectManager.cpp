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

	GameObjectHandleBase GameObjectManager::getObject(UINT64 id) const
	{
		Lock lock(mMutex);

		const auto iterFind = mObjects.find(id);
		if (iterFind != mObjects.end())
			return iterFind->second;

		return nullptr;
	}

	bool GameObjectManager::tryGetObject(UINT64 id, GameObjectHandleBase& object) const
	{
		Lock lock(mMutex);

		const auto iterFind = mObjects.find(id);
		if (iterFind != mObjects.end())
		{
			object = iterFind->second;
			return true;
		}

		return false;
	}

	bool GameObjectManager::objectExists(UINT64 id) const
	{
		Lock lock(mMutex);

		return mObjects.find(id) != mObjects.end();
	}

	void GameObjectManager::remapId(UINT64 oldId, UINT64 newId)
	{
		if (oldId == newId)
			return;

		Lock lock(mMutex);
		mObjects[newId] = mObjects[oldId];
		mObjects.erase(oldId);
	}

	UINT64 GameObjectManager::reserveId()
	{
		return mNextAvailableID.fetch_add(1, std::memory_order_relaxed);
	}

	void GameObjectManager::queueForDestroy(const GameObjectHandleBase& object)
	{
		if (object.isDestroyed())
			return;

		const UINT64 instanceId = object->getInstanceId();
		mQueuedForDestroy[instanceId] = object;
	}

	void GameObjectManager::destroyQueuedObjects()
	{
		for (auto& objPair : mQueuedForDestroy)
			objPair.second->destroyInternal(objPair.second, true);

		mQueuedForDestroy.clear();
	}

	GameObjectHandleBase GameObjectManager::registerObject(const SPtr<GameObject>& object)
	{
		const UINT64 id = mNextAvailableID.fetch_add(1, std::memory_order_relaxed);
		object->initialize(object, id);

		GameObjectHandleBase handle(object);
		{
			Lock lock(mMutex);
			mObjects[id] = handle;
		}

		return handle;
	}

	void GameObjectManager::unregisterObject(GameObjectHandleBase& object)
	{
		{
			Lock lock(mMutex);
			mObjects.erase(object->getInstanceId());
		}

		onDestroyed(static_object_cast<GameObject>(object));
		object.destroy();
	}

	GameObjectDeserializationState::GameObjectDeserializationState(UINT32 options)
		:mOptions(options)
	{ }

	GameObjectDeserializationState::~GameObjectDeserializationState()
	{
		BS_ASSERT(mUnresolvedHandles.empty() && "Deserialization state being destroyed before all handles are resolved.");
		BS_ASSERT(mDeserializedObjects.empty() && "Deserialization state being destroyed before all objects are resolved.");
	}

	void GameObjectDeserializationState::resolve()
	{
		for (auto& entry : mUnresolvedHandles)
		{
			UINT64 instanceId = entry.originalInstanceId;

			bool isInternalReference = false;

			const auto findIter = mIdMapping.find(instanceId);
			if (findIter != mIdMapping.end())
			{
				if ((mOptions & GODM_UseNewIds) != 0)
					instanceId = findIter->second;

				isInternalReference = true;
			}

			if (isInternalReference)
			{
				const auto findIterObj = mDeserializedObjects.find(instanceId);

				if (findIterObj != mDeserializedObjects.end())
					entry.handle.ResolveInternal(findIterObj->second);
				else
				{
					if ((mOptions & GODM_KeepMissing) == 0)
						entry.handle.ResolveInternal(nullptr);
				}
			}
			else if (!isInternalReference && (mOptions & GODM_RestoreExternal) != 0)
			{
				HGameObject obj;
				if(GameObjectManager::instance().tryGetObject(instanceId, obj))
					entry.handle.ResolveInternal(obj);
				else
				{
					if ((mOptions & GODM_KeepMissing) == 0)
						entry.handle.ResolveInternal(nullptr);
				}
			}
			else
			{
				if ((mOptions & GODM_KeepMissing) == 0)
					entry.handle.ResolveInternal(nullptr);
			}
		}

		for (auto iter = mEndCallbacks.rbegin(); iter != mEndCallbacks.rend(); ++iter)
		{
			(*iter)();
		}

		mIdMapping.clear();
		mUnresolvedHandles.clear();
		mEndCallbacks.clear();
		mUnresolvedHandleData.clear();
		mDeserializedObjects.clear();
	}

	void GameObjectDeserializationState::registerUnresolvedHandle(UINT64 originalId, GameObjectHandleBase& object)
	{
		// All handles that are deserialized during a single begin/endDeserialization session pointing to the same object
		// must share the same GameObjectHandleData as that makes certain operations in other systems much simpler.
		// Therefore we store all the unresolved handles, and if a handle pointing to the same object was already
		// processed, or that object was already created we replace the handle's internal GameObjectHandleData.

		// Update the provided handle to ensure all handles pointing to the same object share the same handle data
		bool foundHandleData = false;

		// Search object that are currently being deserialized
		const auto iterFind = mIdMapping.find(originalId);
		if (iterFind != mIdMapping.end())
		{
			const auto iterFind2 = mDeserializedObjects.find(iterFind->second);
			if (iterFind2 != mDeserializedObjects.end())
			{
				object.mData = iterFind2->second.mData;
				foundHandleData = true;
			}
		}

		// Search previously deserialized handles
		if (!foundHandleData)
		{
			auto iterFind = mUnresolvedHandleData.find(originalId);
			if (iterFind != mUnresolvedHandleData.end())
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

	void GameObjectDeserializationState::registerObject(UINT64 originalId, GameObjectHandleBase& object)
	{
		assert(originalId != 0 && "Invalid game object ID.");

		const auto iterFind = mUnresolvedHandleData.find(originalId);
		if (iterFind != mUnresolvedHandleData.end())
		{
			SPtr<GameObject> ptr = object.getInternalPtr();

			object.mData = iterFind->second;
			object.SetHandleDataInternal(ptr);
		}

		const UINT64 newId = object->getInstanceId();
		mIdMapping[originalId] = newId;
		mDeserializedObjects[newId] = object;
	}

	void GameObjectDeserializationState::registerOnDeserializationEndCallback(std::function<void()> callback)
	{
		mEndCallbacks.push_back(callback);
	}
}
