//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCharacterController.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCharacterController.h"
#include "BsScriptControllerColliderCollision.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptControllerControllerCollision.generated.h"

namespace bs
{
	ScriptCCharacterController::OnColliderHitThunkDef ScriptCCharacterController::OnColliderHitThunk;
	ScriptCCharacterController::OnControllerHitThunkDef ScriptCCharacterController::OnControllerHitThunk;

	ScriptCCharacterController::ScriptCCharacterController(MonoObject* managedInstance, const GameObjectHandle<CCharacterController>& value)
		: TScriptComponent(managedInstance, value)
	{
		value->OnColliderHit.Connect(std::bind(&ScriptCCharacterController::OnColliderHit, this, std::placeholders::_1));
		value->OnControllerHit.Connect(std::bind(&ScriptCCharacterController::OnControllerHit, this, std::placeholders::_1));
	}

	void ScriptCCharacterController::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptCCharacterController::InternalMove);
		metaData.ScriptClass->AddInternalCall("Internal_GetFootPosition", (void*)&ScriptCCharacterController::InternalGetFootPosition);
		metaData.ScriptClass->AddInternalCall("Internal_SetFootPosition", (void*)&ScriptCCharacterController::InternalSetFootPosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptCCharacterController::InternalGetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptCCharacterController::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptCCharacterController::InternalGetHeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptCCharacterController::InternalSetHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetUp", (void*)&ScriptCCharacterController::InternalGetUp);
		metaData.ScriptClass->AddInternalCall("Internal_SetUp", (void*)&ScriptCCharacterController::InternalSetUp);
		metaData.ScriptClass->AddInternalCall("Internal_GetClimbingMode", (void*)&ScriptCCharacterController::InternalGetClimbingMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetClimbingMode", (void*)&ScriptCCharacterController::InternalSetClimbingMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetNonWalkableMode", (void*)&ScriptCCharacterController::InternalGetNonWalkableMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetNonWalkableMode", (void*)&ScriptCCharacterController::InternalSetNonWalkableMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinMoveDistance", (void*)&ScriptCCharacterController::InternalGetMinMoveDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetMinMoveDistance", (void*)&ScriptCCharacterController::InternalSetMinMoveDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetContactOffset", (void*)&ScriptCCharacterController::InternalGetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetContactOffset", (void*)&ScriptCCharacterController::InternalSetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetStepOffset", (void*)&ScriptCCharacterController::InternalGetStepOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetStepOffset", (void*)&ScriptCCharacterController::InternalSetStepOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetSlopeLimit", (void*)&ScriptCCharacterController::InternalGetSlopeLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetSlopeLimit", (void*)&ScriptCCharacterController::InternalSetSlopeLimit);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCCharacterController::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCCharacterController::InternalSetLayer);

		OnColliderHitThunk = (OnColliderHitThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnColliderHit", "ControllerColliderCollision&")->GetThunk();
		OnControllerHitThunk = (OnControllerHitThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnControllerHit", "ControllerControllerCollision&")->GetThunk();
	}

	void ScriptCCharacterController::OnColliderHit(const ControllerColliderCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerColliderCollisionInterop interopp0;
		interopp0 = ScriptControllerColliderCollision::ToInterop(p0);
		tmpp0 = ScriptControllerColliderCollision::Box(interopp0);
		MonoUtil::InvokeThunk(OnColliderHitThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCCharacterController::OnControllerHit(const ControllerControllerCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerControllerCollisionInterop interopp0;
		interopp0 = ScriptControllerControllerCollision::ToInterop(p0);
		tmpp0 = ScriptControllerControllerCollision::Box(interopp0);
		MonoUtil::InvokeThunk(OnControllerHitThunk, GetManagedInstance(), tmpp0);
	}

	CharacterCollisionFlag ScriptCCharacterController::InternalMove(ScriptCCharacterController* thisPtr, Vector3* displacement)
	{
		Flags<CharacterCollisionFlag> tmp__output;
		tmp__output = thisPtr->GetHandle()->Move(*displacement);

		CharacterCollisionFlag __output;
		__output = (CharacterCollisionFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalGetFootPosition(ScriptCCharacterController* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFootPosition();

		*__output = tmp__output;
	}

	void ScriptCCharacterController::InternalSetFootPosition(ScriptCCharacterController* thisPtr, Vector3* position)
	{
		thisPtr->GetHandle()->SetFootPosition(*position);
	}

	float ScriptCCharacterController::InternalGetRadius(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetRadius(ScriptCCharacterController* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetRadius(radius);
	}

	float ScriptCCharacterController::InternalGetHeight(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetHeight(ScriptCCharacterController* thisPtr, float height)
	{
		thisPtr->GetHandle()->SetHeight(height);
	}

	void ScriptCCharacterController::InternalGetUp(ScriptCCharacterController* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetUp();

		*__output = tmp__output;
	}

	void ScriptCCharacterController::InternalSetUp(ScriptCCharacterController* thisPtr, Vector3* up)
	{
		thisPtr->GetHandle()->SetUp(*up);
	}

	CharacterClimbingMode ScriptCCharacterController::InternalGetClimbingMode(ScriptCCharacterController* thisPtr)
	{
		CharacterClimbingMode tmp__output;
		tmp__output = thisPtr->GetHandle()->GetClimbingMode();

		CharacterClimbingMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetClimbingMode(ScriptCCharacterController* thisPtr, CharacterClimbingMode mode)
	{
		thisPtr->GetHandle()->SetClimbingMode(mode);
	}

	CharacterNonWalkableMode ScriptCCharacterController::InternalGetNonWalkableMode(ScriptCCharacterController* thisPtr)
	{
		CharacterNonWalkableMode tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNonWalkableMode();

		CharacterNonWalkableMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetNonWalkableMode(ScriptCCharacterController* thisPtr, CharacterNonWalkableMode mode)
	{
		thisPtr->GetHandle()->SetNonWalkableMode(mode);
	}

	float ScriptCCharacterController::InternalGetMinMoveDistance(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMinMoveDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetMinMoveDistance(ScriptCCharacterController* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetMinMoveDistance(value);
	}

	float ScriptCCharacterController::InternalGetContactOffset(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetContactOffset(ScriptCCharacterController* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetContactOffset(value);
	}

	float ScriptCCharacterController::InternalGetStepOffset(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetStepOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetStepOffset(ScriptCCharacterController* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetStepOffset(value);
	}

	void ScriptCCharacterController::InternalGetSlopeLimit(ScriptCCharacterController* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSlopeLimit();

		*__output = tmp__output;
	}

	void ScriptCCharacterController::InternalSetSlopeLimit(ScriptCCharacterController* thisPtr, Radian* value)
	{
		thisPtr->GetHandle()->SetSlopeLimit(*value);
	}

	uint64_t ScriptCCharacterController::InternalGetLayer(ScriptCCharacterController* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::InternalSetLayer(ScriptCCharacterController* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->SetLayer(layer);
	}
} // namespace bs
