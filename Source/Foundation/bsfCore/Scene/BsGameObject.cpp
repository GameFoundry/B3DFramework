//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsGameObject.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "Scene/BsGameObjectManager.h"

namespace bs
{
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