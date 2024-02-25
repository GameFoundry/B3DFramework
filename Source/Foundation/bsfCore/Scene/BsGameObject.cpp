//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsGameObject.h"

#include "BsGameObjectCollection.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "Scene/BsGameObjectManager.h"

using namespace bs;

void GameObject::Initialize(const SPtr<GameObject>& object, u64 instanceId)
{
	mInstanceData = B3DMakeShared<GameObjectInstanceData>();
	mInstanceData->Object = object;
	mInstanceData->MInstanceId = instanceId;
}

void GameObject::SetInstanceData(const SPtr<GameObjectInstanceData>& other)
{
	SPtr<GameObject> myPtr = mInstanceData->Object;
	u64 oldId = mInstanceData->MInstanceId;

	mInstanceData = other;
	mInstanceData->Object = myPtr;

	GameObjectManager::Instance().RemapId(oldId, mInstanceData->MInstanceId);
}

void GameObject::SetOwnerCollection(const SPtr<GameObjectCollection>& collection)
{
	if(!B3D_ENSURE(collection != nullptr))
		return;

	SPtr<GameObjectCollection> currentCollection = mOwnerCollection.lock();
	if(currentCollection == collection)
		return;

	if(B3D_ENSURE(currentCollection != nullptr))
		currentCollection->UnregisterObject(mThisHandle, false);

	collection->RegisterInitializedObject(mThisHandle);
	mOwnerCollection = collection;
}

RTTITypeBase* GameObject::GetRttiStatic()
{
	return GameObjectRTTI::Instance();
}

RTTITypeBase* GameObject::GetRtti() const
{
	return GameObject::GetRttiStatic();
}
