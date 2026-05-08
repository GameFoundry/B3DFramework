//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPrerequisites.h"
#include "Scene/B3DGameObject.h"
#include "Scene/B3DGameObjectHandle.h"
#include "Scene/B3DGameObject.h"
#include "RTTI/B3DGameObjectHandleRTTI.h"

using namespace b3d;

GameObjectHandle::GameObjectHandle(const TShared<GameObject>& object)
{
	B3D_ASSERT(object != nullptr);

	const TShared<GameObjectInstanceData>& instanceData = object->GetInstanceData();
	const UUID id = object->GetId();

	B3D_ASSERT(instanceData != nullptr);
	B3D_ASSERT(id != UUID::kEmpty);

	mSharedHandleData = B3DMakeShared<GameObjectHandleData>(instanceData, id);
}

bool GameObjectHandle::IsDestroyed(bool checkQueued) const
{
	return mSharedHandleData->InstanceData == nullptr ||
		mSharedHandleData->InstanceData->Object == nullptr ||
		(checkQueued && mSharedHandleData->InstanceData->Object->HasGameObjectFlag(GameObjectTransientFlag::QueuedForDestroy));
}

void GameObjectHandle::SetObjectInstanceData(const TShared<GameObject>& object)
{
	B3D_ASSERT(mSharedHandleData != nullptr);
	B3D_ASSERT(object != nullptr);

	const TShared<GameObjectInstanceData>& instanceData = object->GetInstanceData();
	const UUID id = object->GetId();

	B3D_ASSERT(instanceData != nullptr);
	B3D_ASSERT(id != UUID::kEmpty);

	mSharedHandleData->InstanceData = instanceData;
	mSharedHandleData->Id = id;
}

RTTIType* GameObjectHandle::GetRttiStatic()
{
	return GameObjectHandleRTTI::Instance();
}

RTTIType* GameObjectHandle::GetRtti() const
{
	return GameObjectHandle::GetRttiStatic();
}
