//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCollider.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptCollisionData.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMaterial.h"

namespace bs
{
	ScriptColliderBase::OnCollisionBeginThunkDefinition ScriptColliderBase::OnCollisionBeginThunk; 
	ScriptColliderBase::OnCollisionStayThunkDefinition ScriptColliderBase::OnCollisionStayThunk; 
	ScriptColliderBase::OnCollisionEndThunkDefinition ScriptColliderBase::OnCollisionEndThunk; 

	ScriptColliderBase::ScriptColliderBase(MonoObject* managedInstance)
		:ScriptComponentBase(managedInstance)
	 { }

	void ScriptColliderBase::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionBeginThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptColliderBase::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionStayThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptColliderBase::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionEndThunk, GetManagedInstance(), tmpp0);
	}

	ScriptCollider::ScriptCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
		static_cast<GameObjectHandle<CCollider>>(value)->OnCollisionBegin.Connect(std::bind(&ScriptCollider::OnCollisionBegin, this, std::placeholders::_1));
		static_cast<GameObjectHandle<CCollider>>(value)->OnCollisionStay.Connect(std::bind(&ScriptCollider::OnCollisionStay, this, std::placeholders::_1));
		static_cast<GameObjectHandle<CCollider>>(value)->OnCollisionEnd.Connect(std::bind(&ScriptCollider::OnCollisionEnd, this, std::placeholders::_1));
	}

	void ScriptCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetIsTrigger", (void*)&ScriptCollider::InternalSetIsTrigger);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsTrigger", (void*)&ScriptCollider::InternalGetIsTrigger);
		metaData.ScriptClass->AddInternalCall("Internal_SetMass", (void*)&ScriptCollider::InternalSetMass);
		metaData.ScriptClass->AddInternalCall("Internal_GetMass", (void*)&ScriptCollider::InternalGetMass);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptCollider::InternalSetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptCollider::InternalGetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetContactOffset", (void*)&ScriptCollider::InternalSetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetContactOffset", (void*)&ScriptCollider::InternalGetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetRestOffset", (void*)&ScriptCollider::InternalSetRestOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetRestOffset", (void*)&ScriptCollider::InternalGetRestOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCollider::InternalSetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCollider::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_SetCollisionReportMode", (void*)&ScriptCollider::InternalSetCollisionReportMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetCollisionReportMode", (void*)&ScriptCollider::InternalGetCollisionReportMode);

		OnCollisionBeginThunk = (OnCollisionBeginThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
	}

	void ScriptCollider::InternalSetIsTrigger(ScriptColliderBase* self, bool value)
	{
		B3DStaticGameObjectCast<CCollider>(self->GetComponent())->SetIsTrigger(value);
	}

	bool ScriptCollider::InternalGetIsTrigger(ScriptColliderBase* self)
	{
		bool tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(self->GetComponent())->GetIsTrigger();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetMass(ScriptColliderBase* self, float mass)
	{
		B3DStaticGameObjectCast<CCollider>(self->GetComponent())->SetMass(mass);
	}

	float ScriptCollider::InternalGetMass(ScriptColliderBase* self)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(self->GetComponent())->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetMaterial(ScriptColliderBase* self, MonoObject* material)
	{
		TResourceHandle<PhysicsMaterial> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::ToNative(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<PhysicsMaterial>(scriptObjectWrappermaterial->GetHandle());
		B3DStaticGameObjectCast<CCollider>(self->GetComponent())->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptCollider::InternalGetMaterial(ScriptColliderBase* self)
	{
		TResourceHandle<PhysicsMaterial> tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(self->GetComponent())->GetMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCollider::InternalSetContactOffset(ScriptColliderBase* self, float value)
	{
		B3DStaticGameObjectCast<CCollider>(self->GetComponent())->SetContactOffset(value);
	}

	float ScriptCollider::InternalGetContactOffset(ScriptColliderBase* self)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(self->GetComponent())->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetRestOffset(ScriptColliderBase* self, float value)
	{
		B3DStaticGameObjectCast<CCollider>(self->GetComponent())->SetRestOffset(value);
	}

	float ScriptCollider::InternalGetRestOffset(ScriptColliderBase* self)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(self->GetComponent())->GetRestOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetLayer(ScriptColliderBase* self, uint64_t layer)
	{
		B3DStaticGameObjectCast<CCollider>(self->GetComponent())->SetLayer(layer);
	}

	uint64_t ScriptCollider::InternalGetLayer(ScriptColliderBase* self)
	{
		uint64_t tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(self->GetComponent())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetCollisionReportMode(ScriptColliderBase* self, CollisionReportMode mode)
	{
		B3DStaticGameObjectCast<CCollider>(self->GetComponent())->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCollider::InternalGetCollisionReportMode(ScriptColliderBase* self)
	{
		CollisionReportMode tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(self->GetComponent())->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}
}
