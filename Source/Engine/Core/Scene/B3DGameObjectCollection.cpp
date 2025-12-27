//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DGameObjectCollection.h"

#include "B3DGameObjectManager.h"
#include "Scene/B3DGameObject.h"

using namespace b3d;

GameObjectCollection::GameObjectCollection(PrivatelyConstruct)
	:mId(UUIDGenerator::GenerateRandom())
{ }

GameObjectCollection::~GameObjectCollection()
{
	DestroyQueuedObjects();

	GameObjectManager::Instance().UnregisterGameObjectCollection(*this);
}

GameObjectHandleBase GameObjectCollection::RegisterNewObject(const SPtr<GameObject>& object)
{
	if(!B3D_ENSURE(object != nullptr))
		return nullptr;

	object->InitializeInstanceData(object);

	const UUID& id = object->GetId();
	B3D_ASSERT(!id.Empty());

	if(auto found = mObjects.find(id); !B3D_ENSURE(found == mObjects.end()))
		return nullptr;

	GameObjectHandleBase handle(object);

	if(mHandleResolveActive)
	{
		// Handle could have been created before the object, check that and re-use the shared handle data.
		// All handles created during a single resolve operation must use the same shared handle data, so we can patch the single instance of it during resolve.
		if(auto found = mUnresolvedHandleSharedHandleData.find(handle.GetId()); found != mUnresolvedHandleSharedHandleData.end())
		{
			handle = GameObjectHandleBase(found->second);
			handle.SetObjectInstanceData(object);
		}
		// Handle hasn't been registered yet, store its shared handle data for when it does get registered.
		else
		{
			mUnresolvedHandleSharedHandleData[handle.GetId()] = handle.GetSharedHandleData();
		}
	}

	mObjects[object->GetId()] = handle;

	B3D_ENSURE(object->mOwnerCollection.lock() == nullptr);
	object->mOwnerCollection = shared_from_this();

	// Object was destroyed and then re-created without calling DestroyQueuedObjects(). This is not supported.
	B3D_ENSURE(mQueuedForDestroy.find(object->GetId()) == mQueuedForDestroy.end());

	return handle;
}

void GameObjectCollection::RegisterExistingObject(const GameObjectHandleBase& handle)
{
	if(!B3D_ENSURE(handle.IsValid()))
		return;

	if(auto found = mObjects.find(handle.GetId()); !B3D_ENSURE(found == mObjects.end()))
		return;

	mObjects[handle->GetId()] = handle;

	B3D_ENSURE(handle->mOwnerCollection.lock() == nullptr);
	handle->mOwnerCollection = shared_from_this();

	// Object was destroyed and then re-created without calling DestroyQueuedObjects(). This is not supported.
	B3D_ENSURE(mQueuedForDestroy.find(handle->GetId()) == mQueuedForDestroy.end());
}

void GameObjectCollection::UnregisterObject(GameObjectHandleBase& object, bool triggerDestroyEvent)
{
	if(!B3D_ENSURE(!object.IsDestroyed(false)))
		return;

	if(!B3D_ENSURE((object->mOwnerCollection.lock().get() == this)))
		return;

	mObjects.erase(object.GetId());
	object->mOwnerCollection.reset();

	if(triggerDestroyEvent)
		OnDestroyed(B3DStaticGameObjectCast<GameObject>(object));
}

GameObjectHandleBase GameObjectCollection::GetObject(const UUID& id) const
{
	if(const auto found = mObjects.find(id); found != mObjects.end())
		return found->second;

	return nullptr;
}

bool GameObjectCollection::TryGetObject(const UUID& uuid, GameObjectHandleBase& object) const
{
	const auto iterFind = mObjects.find(uuid);
	if(iterFind != mObjects.end())
	{
		object = iterFind->second;
		return true;
	}

	return false;
}

bool GameObjectCollection::ObjectExists(const UUID& uuid) const
{
	return mObjects.find(uuid) != mObjects.end();
}

void GameObjectCollection::ReplaceGameObjectInstance(GameObjectHandleBase& newObjectHandle, const SPtr<GameObjectInstanceData>& originalObjectInstanceData)
{
	B3D_ASSERT(originalObjectInstanceData != nullptr);

	const UUID originalObjectId = newObjectHandle.GetId();
	const SPtr<GameObject> newObject = newObjectHandle.GetShared();

	newObjectHandle->SetInstanceData(originalObjectInstanceData);
	newObjectHandle.SetObjectInstanceData(newObject);

	const UUID newObjectId = newObjectHandle.GetId();
	if(originalObjectId != newObjectId)
	{
		if(auto found = mObjects.find(originalObjectId); found != mObjects.end())
		{
			GameObjectHandleBase handle = found->second;

			mObjects.erase(found);
			mObjects[newObjectId] = handle;

			// Object was destroyed and then re-created without calling DestroyQueuedObjects(). This is not supported.
			B3D_ENSURE(mQueuedForDestroy.find(newObjectId) == mQueuedForDestroy.end());
		}
	}
}

void GameObjectCollection::ChangeGameObjectId(GameObjectHandleBase& gameObject, const UUID& newId)
{
	const UUID originalObjectId = gameObject.GetId();

	if(originalObjectId != newId)
	{
		if(!gameObject.IsDestroyed(false))
			gameObject->SetId(newId);

		gameObject.GetSharedHandleData()->Id = newId;

		if(auto found = mObjects.find(originalObjectId); found != mObjects.end())
		{
			GameObjectHandleBase handle = found->second;

			mObjects.erase(found);
			mObjects[newId] = handle;

			// Object was destroyed and then re-created without calling DestroyQueuedObjects(). This is not supported.
			B3D_ENSURE(mQueuedForDestroy.find(newId) == mQueuedForDestroy.end());
		}
	}
}

void GameObjectCollection::QueueForDestroy(const GameObjectHandleBase& object)
{
	if(object.IsDestroyed(false))
		return;

	auto result = mQueuedForDestroy.insert(std::make_pair(object.GetId(), object));
	if(B3D_ENSURE(result.second))
		mOrderedQueuedForDestroy.push_back(object.GetId());
}

void GameObjectCollection::DestroyQueuedObjects()
{
	for(const auto& id : mOrderedQueuedForDestroy)
	{
		auto found = mQueuedForDestroy.find(id);
		if(!B3D_ENSURE(found != mQueuedForDestroy.end()))
			continue;

		GameObjectHandleBase handle = found->second;
		mQueuedForDestroy.erase(found);

		if(handle.IsDestroyed(false))
			continue;

		handle->DestroyImmediate();
	}

	mQueuedForDestroy.clear();
	mOrderedQueuedForDestroy.clear();
}

void GameObjectCollection::RegisterUnresolvedHandle(GameObjectHandleBase& handle)
{
	if(!B3D_ENSURE(mHandleResolveActive))
		return;

	if(handle.GetId().Empty())
		return;

	// Objects are allowed to update ids during deserialization, but handles pointing to those objects will still have the old ids, so make sure to remap them.
	// It's possible the object has not yet been deserialized, and therefore hasn't registered an id remapping yet. In which case we store using the
	// old id and perform the remapping when it is registered.
	UUID remappedUUID;
	if(auto found = mUUIDRemapping.find(handle.GetId()); found != mUUIDRemapping.end())
		remappedUUID = found->second;
	else
		remappedUUID = handle.GetId();

	// Make sure the handle uses the existing shared handle data, if it exists. See RegisterAndInitializeObject
	if(auto found = mUnresolvedHandleSharedHandleData.find(remappedUUID); found != mUnresolvedHandleSharedHandleData.end())
		handle = found->second;
	else
		mUnresolvedHandleSharedHandleData[remappedUUID] = handle.GetSharedHandleData();

	mUnresolvedHandles.push_back(handle);
}

void GameObjectCollection::RegisterUnresolvedHandleIdRemapping(const UUID& originalId, const UUID& newId)
{
	if(!B3D_ENSURE(mHandleResolveActive))
		return;

	if(originalId == newId)
		return;

	mUUIDRemapping[originalId] = newId;

	// It's possible handle data was registered with the old id (in case the handle was deserialized before the game object). In that
	// case make sure to update it to the new id.
	if(auto found = mUnresolvedHandleSharedHandleData.find(originalId); found != mUnresolvedHandleSharedHandleData.end())
	{
		mUnresolvedHandleSharedHandleData[newId] = found->second;
		mUnresolvedHandleSharedHandleData.erase(originalId);
	}
}

void GameObjectCollection::BeginHandleResolve()
{
	if(!B3D_ENSURE(!mHandleResolveActive))
		return;

	B3D_ASSERT(mUUIDRemapping.empty());
	B3D_ASSERT(mUnresolvedHandles.empty());
	B3D_ASSERT(mUnresolvedHandleSharedHandleData.empty());

	mHandleResolveActive = true;
}

void GameObjectCollection::EndHandleResolve()
{
	if(!B3D_ENSURE(mHandleResolveActive))
		return;

	for(auto& unresolvedHandle : mUnresolvedHandles)
	{
		UUID remappedUUID;
		if(auto found = mUUIDRemapping.find(unresolvedHandle.GetId()); found != mUUIDRemapping.end())
			remappedUUID = found->second;
		else
			remappedUUID = unresolvedHandle.GetId();

		const auto foundObject = mObjects.find(remappedUUID);
		if(foundObject == mObjects.end())
			continue;

		unresolvedHandle.SetObjectInstanceData(foundObject->second);
		B3D_ASSERT(remappedUUID == unresolvedHandle.GetId());
	}

	mUUIDRemapping.clear();
	mUnresolvedHandles.clear();
	mUnresolvedHandleSharedHandleData.clear();

	mHandleResolveActive = false;
}

SPtr<GameObjectCollection> GameObjectCollection::Create()
{
	SPtr<GameObjectCollection> collection = B3DMakeShared<GameObjectCollection>(PrivatelyConstruct());

	if(GameObjectManager::IsStarted()) // May not be started for default object
		GameObjectManager::Instance().RegisterGameObjectCollection(collection);

	return collection;
}

