//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsGameObjectCollection.h"

#include "BsGameObjectManager.h"
#include "Scene/BsGameObject.h"

using namespace bs;

GameObjectCollection::~GameObjectCollection()
{
	DestroyQueuedObjects();
}

GameObjectHandleBase GameObjectCollection::RegisterAndInitializeObject(const SPtr<GameObject>& object)
{
	if(!B3D_ENSURE(object != nullptr))
		return nullptr;

	object->Initialize(object, 0);

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
			handle.SetObject(object);
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

	return handle;
}

void GameObjectCollection::RegisterInitializedObject(const GameObjectHandleBase& handle)
{
	if(!B3D_ENSURE(handle.IsValid()))
		return;

	if(auto found = mObjects.find(handle.GetId()); !B3D_ENSURE(found == mObjects.end()))
		return;

	mObjects[handle->GetId()] = handle;

	B3D_ENSURE(handle->mOwnerCollection.lock() == nullptr);
	handle->mOwnerCollection = shared_from_this();
}

void GameObjectCollection::UnregisterObject(GameObjectHandleBase& object, bool triggerDestroyEvent)
{
	if(!B3D_ENSURE(object.IsValid()))
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

	const SPtr<GameObject> newObject = newObjectHandle.GetShared();
	B3D_ENSURE(newObject == nullptr || newObjectHandle.GetId() == newObject->GetId());

	newObjectHandle->SetInstanceData(originalObjectInstanceData);
	newObjectHandle.SetObject(newObject);
}

void GameObjectCollection::QueueForDestroy(const GameObjectHandleBase& object)
{
	if(object.IsDestroyed())
		return;

	mQueuedForDestroy[object.GetId()] = object;
}

void GameObjectCollection::DestroyQueuedObjects()
{
	for(auto it = mQueuedForDestroy.begin(); it != mQueuedForDestroy.end();)
	{
		GameObjectHandleBase handle = it->second;
		it = mQueuedForDestroy.erase(it);

		if(handle.IsDestroyed())
			continue;

		handle->DestroyInternal(handle, true);
	}

	mQueuedForDestroy.clear();
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
	if(!B3D_ENSURE(!mHandleResolveActive))
		return;

	for(auto& unresolvedHandle : mUnresolvedHandles)
	{
		UUID remappedUUID;
		if(auto found = mUUIDRemapping.find(unresolvedHandle.GetId()); B3D_ENSURE(found != mUUIDRemapping.end()))
			remappedUUID = found->second;
		else
			continue;

		const auto foundObject = mObjects.find(remappedUUID);
		if(foundObject == mObjects.end())
			continue;

		unresolvedHandle.SetObject(foundObject->second);
		B3D_ASSERT(remappedUUID == unresolvedHandle.GetId());
	}

	mUUIDRemapping.clear();
	mUnresolvedHandles.clear();
	mUnresolvedHandleSharedHandleData.clear();

	mHandleResolveActive = false;
}
