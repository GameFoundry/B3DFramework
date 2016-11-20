//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGameObject.h"
#include "BsGameObjectRTTI.h"
#include "BsGameObjectManager.h"

namespace bs
{
	GameObject::GameObject()
		:mLinkId((UINT32)-1), mIsDestroyed(false)
	{ }

	GameObject::~GameObject()
	{ }

	void GameObject::initialize(const SPtr<GameObject>& object, UINT64 instanceId)
	{
		mInstanceData = bs_shared_ptr_new<GameObjectInstanceData>();
		mInstanceData->object = object;
		mInstanceData->mInstanceId = instanceId;
	}

	void GameObject::_setInstanceData(GameObjectInstanceDataPtr& other)
	{
		SPtr<GameObject> myPtr = mInstanceData->object;
		UINT64 oldId = mInstanceData->mInstanceId;

		mInstanceData = other;
		mInstanceData->object = myPtr;

		GameObjectManager::instance().remapId(oldId, mInstanceData->mInstanceId);
	}
	
	RTTITypeBase* GameObject::getRTTIStatic()
	{
		return GameObjectRTTI::instance();
	}

	RTTITypeBase* GameObject::getRTTI() const
	{
		return GameObject::getRTTIStatic();
	}
}