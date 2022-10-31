//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Scene/BsGameObject.h"
#include "Scene/BsGameObjectHandle.h"
#include "Error/BsException.h"
#include "Scene/BsGameObject.h"
#include "Private/RTTI/BsGameObjectHandleRTTI.h"

using namespace bs;

GameObjectHandleBase::GameObjectHandleBase(const SPtr<GameObject>& ptr)
{
	mData = B3DMakeShared<GameObjectHandleData>(ptr->mInstanceData);
}

bool GameObjectHandleBase::IsDestroyed(bool checkQueued) const
{
	return mData->MPtr == nullptr || mData->MPtr->Object == nullptr || (checkQueued && mData->MPtr->Object->GetIsDestroyedInternal());
}

void GameObjectHandleBase::SetHandleDataInternal(const SPtr<GameObject>& object)
{
	mData->MPtr = object->mInstanceData;
}

void GameObjectHandleBase::ThrowIfDestroyed() const
{
	if(IsDestroyed())
	{
		BS_EXCEPT(InternalErrorException, "Trying to access an object that has been destroyed.");
	}
}

RTTITypeBase* GameObjectHandleBase::GetRttiStatic()
{
	return GameObjectHandleRTTI::Instance();
}

RTTITypeBase* GameObjectHandleBase::GetRtti() const
{
	return GameObjectHandleBase::GetRttiStatic();
}
