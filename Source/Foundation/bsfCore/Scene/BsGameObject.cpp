//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsGameObject.h"
#include "BsGameObjectCollection.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

using namespace bs;

void GameObject::Initialize(const SPtr<GameObject>& object)
{
	mInstanceData = B3DMakeShared<GameObjectInstanceData>();
	mInstanceData->Object = object;
}

void GameObject::SetInstanceData(const SPtr<GameObjectInstanceData>& other)
{
	SPtr<GameObject> object = mInstanceData->Object;

	mInstanceData = other;
	mInstanceData->Object = nullptr; // Note: Important to clear this before assign below, because GameObject destructor will clear mInstanceData->Object, which will trigger if this is the last object instance
	mInstanceData->Object = object;
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

void GameObject::DestroyImmediate()
{
	mInstanceData->Object = nullptr;
}

RTTITypeBase* GameObject::GetRttiStatic()
{
	return GameObjectRTTI::Instance();
}

RTTITypeBase* GameObject::GetRtti() const
{
	return GameObject::GetRttiStatic();
}
