//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCharacterController.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCharacterController.h"
#include "BsScriptControllerControllerCollision.generated.h"
#include "BsScriptTVector3.generated.h"
#include "BsScriptControllerColliderCollision.generated.h"

namespace bs
{
	ScriptCharacterController::OnColliderHitThunkDefinition ScriptCharacterController::OnColliderHitThunk; 
	ScriptCharacterController::OnControllerHitThunkDefinition ScriptCharacterController::OnControllerHitThunk; 

	ScriptCharacterController::ScriptCharacterController(const GameObjectHandle<CCharacterController>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptCharacterController::~ScriptCharacterController()
	{
		UnregisterEvents();
	}

	void ScriptCharacterController::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptCharacterController::InternalMove);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFootPosition", (void*)&ScriptCharacterController::InternalGetFootPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFootPosition", (void*)&ScriptCharacterController::InternalSetFootPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptCharacterController::InternalGetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptCharacterController::InternalSetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptCharacterController::InternalGetHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptCharacterController::InternalSetHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUp", (void*)&ScriptCharacterController::InternalGetUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUp", (void*)&ScriptCharacterController::InternalSetUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClimbingMode", (void*)&ScriptCharacterController::InternalGetClimbingMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetClimbingMode", (void*)&ScriptCharacterController::InternalSetClimbingMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNonWalkableMode", (void*)&ScriptCharacterController::InternalGetNonWalkableMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNonWalkableMode", (void*)&ScriptCharacterController::InternalSetNonWalkableMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinMoveDistance", (void*)&ScriptCharacterController::InternalGetMinMoveDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMinMoveDistance", (void*)&ScriptCharacterController::InternalSetMinMoveDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetContactOffset", (void*)&ScriptCharacterController::InternalGetContactOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetContactOffset", (void*)&ScriptCharacterController::InternalSetContactOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetStepOffset", (void*)&ScriptCharacterController::InternalGetStepOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetStepOffset", (void*)&ScriptCharacterController::InternalSetStepOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSlopeLimit", (void*)&ScriptCharacterController::InternalGetSlopeLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSlopeLimit", (void*)&ScriptCharacterController::InternalSetSlopeLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCharacterController::InternalGetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCharacterController::InternalSetLayer);

		OnColliderHitThunk = (OnColliderHitThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnColliderHit", "ControllerColliderCollision&")->GetThunk();
		OnControllerHitThunk = (OnControllerHitThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnControllerHit", "ControllerControllerCollision&")->GetThunk();
	}

	MonoObject* ScriptCharacterController::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptCharacterController::OnColliderHit(const ControllerColliderCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerColliderCollisionInterop interopp0;
		interopp0 = ScriptControllerColliderCollision::ToInterop(p0);
		tmpp0 = ScriptControllerColliderCollision::Box(interopp0);
		MonoUtil::InvokeThunk(OnColliderHitThunk, GetScriptObject(), tmpp0);
	}

	void ScriptCharacterController::OnControllerHit(const ControllerControllerCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerControllerCollisionInterop interopp0;
		interopp0 = ScriptControllerControllerCollision::ToInterop(p0);
		tmpp0 = ScriptControllerControllerCollision::Box(interopp0);
		MonoUtil::InvokeThunk(OnControllerHitThunk, GetScriptObject(), tmpp0);
	}

	void ScriptCharacterController::RegisterEvents()
	{
		OnColliderHitConnection = static_cast<CCharacterController*>(GetNativeObject())->OnColliderHit.Connect(std::bind(&ScriptCharacterController::OnColliderHit, this, std::placeholders::_1));
		OnControllerHitConnection = static_cast<CCharacterController*>(GetNativeObject())->OnControllerHit.Connect(std::bind(&ScriptCharacterController::OnControllerHit, this, std::placeholders::_1));
	}
	void ScriptCharacterController::UnregisterEvents()
	{
		OnColliderHitConnection.Disconnect();
		OnControllerHitConnection.Disconnect();
	}
	CharacterCollisionFlag ScriptCharacterController::InternalMove(ScriptCharacterController* self, TVector3<float>* displacement)
	{
		Flags<CharacterCollisionFlag> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->Move(*displacement);

		CharacterCollisionFlag __output;
		__output = (CharacterCollisionFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalGetFootPosition(ScriptCharacterController* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetFootPosition();

		*__output = tmp__output;
	}

	void ScriptCharacterController::InternalSetFootPosition(ScriptCharacterController* self, TVector3<float>* position)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetFootPosition(*position);
	}

	float ScriptCharacterController::InternalGetRadius(ScriptCharacterController* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetRadius(ScriptCharacterController* self, float radius)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetRadius(radius);
	}

	float ScriptCharacterController::InternalGetHeight(ScriptCharacterController* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetHeight(ScriptCharacterController* self, float height)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetHeight(height);
	}

	void ScriptCharacterController::InternalGetUp(ScriptCharacterController* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetUp();

		*__output = tmp__output;
	}

	void ScriptCharacterController::InternalSetUp(ScriptCharacterController* self, TVector3<float>* up)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetUp(*up);
	}

	CharacterClimbingMode ScriptCharacterController::InternalGetClimbingMode(ScriptCharacterController* self)
	{
		CharacterClimbingMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetClimbingMode();

		CharacterClimbingMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetClimbingMode(ScriptCharacterController* self, CharacterClimbingMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetClimbingMode(mode);
	}

	CharacterNonWalkableMode ScriptCharacterController::InternalGetNonWalkableMode(ScriptCharacterController* self)
	{
		CharacterNonWalkableMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetNonWalkableMode();

		CharacterNonWalkableMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetNonWalkableMode(ScriptCharacterController* self, CharacterNonWalkableMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetNonWalkableMode(mode);
	}

	float ScriptCharacterController::InternalGetMinMoveDistance(ScriptCharacterController* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetMinMoveDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetMinMoveDistance(ScriptCharacterController* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetMinMoveDistance(value);
	}

	float ScriptCharacterController::InternalGetContactOffset(ScriptCharacterController* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetContactOffset(ScriptCharacterController* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetContactOffset(value);
	}

	float ScriptCharacterController::InternalGetStepOffset(ScriptCharacterController* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetStepOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetStepOffset(ScriptCharacterController* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetStepOffset(value);
	}

	void ScriptCharacterController::InternalGetSlopeLimit(ScriptCharacterController* self, TRadian<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TRadian<float> tmp__output;
		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetSlopeLimit();

		*__output = tmp__output;
	}

	void ScriptCharacterController::InternalSetSlopeLimit(ScriptCharacterController* self, TRadian<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetSlopeLimit(*value);
	}

	uint64_t ScriptCharacterController::InternalGetLayer(ScriptCharacterController* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCharacterController*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCharacterController::InternalSetLayer(ScriptCharacterController* self, uint64_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCharacterController*>(self->GetNativeObject())->SetLayer(layer);
	}
}
