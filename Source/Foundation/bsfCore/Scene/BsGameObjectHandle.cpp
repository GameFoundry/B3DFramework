//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCorePrerequisites.h"
#include "Scene/BsGameObject.h"
#include "Scene/BsGameObjectHandle.h"
#include "Scene/BsGameObject.h"
#include "Private/RTTI/BsGameObjectHandleRTTI.h"

using namespace bs;

GameObjectHandleBase::GameObjectHandleBase(const SPtr<GameObject>& object)
{
	B3D_ASSERT(object != nullptr);

	const SPtr<GameObjectInstanceData>& instanceData = object->GetInstanceData();
	const UUID id = object->GetId();

	B3D_ASSERT(instanceData != nullptr);
	B3D_ASSERT(id != UUID::kEmpty);

	mSharedHandleData = B3DMakeShared<GameObjectHandleData>(instanceData, id);
}

bool GameObjectHandleBase::IsDestroyed(bool checkQueued) const
{
	return mSharedHandleData->InstanceData == nullptr || mSharedHandleData->InstanceData->Object == nullptr || (checkQueued && mSharedHandleData->InstanceData->Object->GetIsDestroyedInternal());
}

void GameObjectHandleBase::SetSharedHandleData(const SPtr<GameObject>& object)
{
	B3D_ASSERT(mSharedHandleData != nullptr);
	B3D_ASSERT(object != nullptr);

	const SPtr<GameObjectInstanceData>& instanceData = object->GetInstanceData();
	const UUID id = object->GetId();

	B3D_ASSERT(instanceData != nullptr);
	B3D_ASSERT(id != UUID::kEmpty);

	mSharedHandleData->InstanceData = instanceData;
	mSharedHandleData->Id = id;
			
}

RTTITypeBase* GameObjectHandleBase::GetRttiStatic()
{
	return GameObjectHandleRTTI::Instance();
}

RTTITypeBase* GameObjectHandleBase::GetRtti() const
{
	return GameObjectHandleBase::GetRttiStatic();
}
