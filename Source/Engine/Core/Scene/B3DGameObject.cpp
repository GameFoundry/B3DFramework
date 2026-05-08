//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DGameObject.h"
#include "B3DGameObjectCollection.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "Script/B3DIScriptObjectWrapper.h"

using namespace b3d;

void GameObject::InitializeInstanceData(const TShared<GameObject>& object)
{
	mInstanceData = B3DMakeShared<GameObjectInstanceData>();
	mInstanceData->Object = object;
}

void GameObject::SetInstanceData(const TShared<GameObjectInstanceData>& other)
{
	TShared<GameObject> object = mInstanceData->Object;

	mInstanceData = other;
	mInstanceData->Object = nullptr; // Note: Important to clear this before assign below, because GameObject destructor will clear mInstanceData->Object, which will trigger if this is the last object instance
	mInstanceData->Object = object;
}

void GameObject::SetOwnerCollection(const TShared<GameObjectCollection>& collection)
{
	if(!B3D_ENSURE(collection != nullptr))
		return;

	TShared<GameObjectCollection> currentCollection = mOwnerCollection.lock();
	if(currentCollection == collection)
		return;

	if(B3D_ENSURE(currentCollection != nullptr))
		currentCollection->UnregisterObject(mThisHandle, false);

	collection->RegisterExistingObject(mThisHandle);
	mOwnerCollection = collection;
}

void GameObject::DestroyImmediate()
{
	if(IScriptObjectWrapper* scriptObjectWrapper = GetScriptObjectWrapper())
		scriptObjectWrapper->NotifyNativeObjectDestroyed();

	ClearAssociatedScriptObjectWrapper();

	const TShared<GameObjectCollection>& ownerCollection = mOwnerCollection.lock();
	if(ownerCollection != nullptr) // Allowed to be null during GameObjectCollection destructor call
		ownerCollection->UnregisterObject(mThisHandle, HasGameObjectFlag(GameObjectTransientFlag::Initialized));

	mInstanceData->Object = nullptr;

	SetGameObjectFlag(GameObjectTransientFlag::Destroyed);
}

void GameObject::QueueForDestroy()
{
	const TShared<GameObjectCollection>& ownerCollection = mOwnerCollection.lock();
	if(ownerCollection != nullptr) // Allowed to be null during GameObjectCollection destructor call
		ownerCollection->QueueForDestroy(mThisHandle);

	SetGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy);
}

RTTIType* GameObject::GetRttiStatic()
{
	return GameObjectRTTI::Instance();
}

RTTIType* GameObject::GetRtti() const
{
	return GameObject::GetRttiStatic();
}
