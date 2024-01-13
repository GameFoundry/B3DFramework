//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCharacterController.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCharacterController.h"
#include "BsScriptControllerControllerCollision.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptControllerColliderCollision.generated.h"

namespace bs
{
	ScriptCharacterController::OnColliderHitThunkDef ScriptCharacterController::OnColliderHitThunk; 
	ScriptCharacterController::OnControllerHitThunkDef ScriptCharacterController::OnControllerHitThunk; 

	ScriptCharacterController::ScriptCharacterController(MonoObject* managedInstance, const GameObjectHandle<CCharacterController>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->OnColliderHit.Connect(std::bind(&ScriptCharacterController::OnColliderHit, this, std::placeholders::_1));
		value->OnControllerHit.Connect(std::bind(&ScriptCharacterController::OnControllerHit, this, std::placeholders::_1));
	}

	void ScriptCharacterController::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptCharacterController::InternalMove);
		metaData.ScriptClass->AddInternalCall("Internal_GetFootPosition", (void*)&ScriptCharacterController::InternalGetFootPosition);
		metaData.ScriptClass->AddInternalCall("Internal_SetFootPosition", (void*)&ScriptCharacterController::InternalSetFootPosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptCharacterController::InternalGetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptCharacterController::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptCharacterController::InternalGetHeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptCharacterController::InternalSetHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetUp", (void*)&ScriptCharacterController::InternalGetUp);
		metaData.ScriptClass->AddInternalCall("Internal_SetUp", (void*)&ScriptCharacterController::InternalSetUp);
		metaData.ScriptClass->AddInternalCall("Internal_GetClimbingMode", (void*)&ScriptCharacterController::InternalGetClimbingMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetClimbingMode", (void*)&ScriptCharacterController::InternalSetClimbingMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetNonWalkableMode", (void*)&ScriptCharacterController::InternalGetNonWalkableMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetNonWalkableMode", (void*)&ScriptCharacterController::InternalSetNonWalkableMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinMoveDistance", (void*)&ScriptCharacterController::InternalGetMinMoveDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetMinMoveDistance", (void*)&ScriptCharacterController::InternalSetMinMoveDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetContactOffset", (void*)&ScriptCharacterController::InternalGetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetContactOffset", (void*)&ScriptCharacterController::InternalSetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetStepOffset", (void*)&ScriptCharacterController::InternalGetStepOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetStepOffset", (void*)&ScriptCharacterController::InternalSetStepOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetSlopeLimit", (void*)&ScriptCharacterController::InternalGetSlopeLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetSlopeLimit", (void*)&ScriptCharacterController::InternalSetSlopeLimit);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCharacterController::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCharacterController::InternalSetLayer);

		OnColliderHitThunk = (OnColliderHitThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnColliderHit", "ControllerColliderCollision&")->GetThunk();
		OnControllerHitThunk = (OnControllerHitThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnControllerHit", "ControllerControllerCollision&")->GetThunk();
	}

	void ScriptCharacterController::OnColliderHit(const ControllerColliderCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerColliderCollisionInterop interopp0;
		interopp0 = ScriptControllerColliderCollision::ToInterop(p0);
		tmpp0 = ScriptControllerColliderCollision::Box(interopp0);
		MonoUtil::InvokeThunk(OnColliderHitThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCharacterController::OnControllerHit(const ControllerControllerCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerControllerCollisionInterop interopp0;
		interopp0 = ScriptControllerControllerCollision::ToInterop(p0);
		tmpp0 = ScriptControllerControllerCollision::Box(interopp0);
		MonoUtil::InvokeThunk(OnControllerHitThunk, GetManagedInstance(), tmpp0);
	}

	CharacterCollisionFlag ScriptCharacterController::InternalMove(ScriptCharacterController* thisPtr, TVector3<float>* displacement)
	{
		Flags<CharacterCollisionFlag> tmp__output;
		tmp__output = thisPtr->GetHandle()->Move(*displacement);

		CharacterCollisionFlag __output;
		__output = (CharacterCollisionFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalGetFootPosition(ScriptCharacterController* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFootPosition();

		*__output = tmp__output;
	}

	void ScriptCharacterController::InternalSetFootPosition(ScriptCharacterController* thisPtr, TVector3<float>* position)
	{
		thisPtr->GetHandle()->SetFootPosition(*position);
	}

	float ScriptCharacterController::InternalGetRadius(ScriptCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetRadius(ScriptCharacterController* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetRadius(radius);
	}

	float ScriptCharacterController::InternalGetHeight(ScriptCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetHeight(ScriptCharacterController* thisPtr, float height)
	{
		thisPtr->GetHandle()->SetHeight(height);
	}

	void ScriptCharacterController::InternalGetUp(ScriptCharacterController* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetUp();

		*__output = tmp__output;
	}

	void ScriptCharacterController::InternalSetUp(ScriptCharacterController* thisPtr, TVector3<float>* up)
	{
		thisPtr->GetHandle()->SetUp(*up);
	}

	CharacterClimbingMode ScriptCharacterController::InternalGetClimbingMode(ScriptCharacterController* thisPtr)
	{
		CharacterClimbingMode tmp__output;
		tmp__output = thisPtr->GetHandle()->GetClimbingMode();

		CharacterClimbingMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetClimbingMode(ScriptCharacterController* thisPtr, CharacterClimbingMode mode)
	{
		thisPtr->GetHandle()->SetClimbingMode(mode);
	}

	CharacterNonWalkableMode ScriptCharacterController::InternalGetNonWalkableMode(ScriptCharacterController* thisPtr)
	{
		CharacterNonWalkableMode tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNonWalkableMode();

		CharacterNonWalkableMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetNonWalkableMode(ScriptCharacterController* thisPtr, CharacterNonWalkableMode mode)
	{
		thisPtr->GetHandle()->SetNonWalkableMode(mode);
	}

	float ScriptCharacterController::InternalGetMinMoveDistance(ScriptCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMinMoveDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetMinMoveDistance(ScriptCharacterController* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetMinMoveDistance(value);
	}

	float ScriptCharacterController::InternalGetContactOffset(ScriptCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetContactOffset(ScriptCharacterController* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetContactOffset(value);
	}

	float ScriptCharacterController::InternalGetStepOffset(ScriptCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetStepOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetStepOffset(ScriptCharacterController* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetStepOffset(value);
	}

	void ScriptCharacterController::InternalGetSlopeLimit(ScriptCharacterController* thisPtr, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSlopeLimit();

		*__output = tmp__output;
	}

	void ScriptCharacterController::InternalSetSlopeLimit(ScriptCharacterController* thisPtr, TRadian<float>* value)
	{
		thisPtr->GetHandle()->SetSlopeLimit(*value);
	}

	uint64_t ScriptCharacterController::InternalGetLayer(ScriptCharacterController* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetLayer(ScriptCharacterController* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->SetLayer(layer);
	}
}
