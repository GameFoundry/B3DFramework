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
	ScriptCColliderBase::ScriptCColliderBase(MonoObject* managedInstance)
		:ScriptComponentBase(managedInstance)
	 { }

	ScriptCCollider::OnCollisionBeginThunkDef ScriptCCollider::OnCollisionBeginThunk; 
	ScriptCCollider::OnCollisionStayThunkDef ScriptCCollider::OnCollisionStayThunk; 
	ScriptCCollider::OnCollisionEndThunkDef ScriptCCollider::OnCollisionEndThunk; 

	ScriptCCollider::ScriptCCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->OnCollisionBegin.Connect(std::bind(&ScriptCCollider::OnCollisionBegin, this, std::placeholders::_1));
		value->OnCollisionStay.Connect(std::bind(&ScriptCCollider::OnCollisionStay, this, std::placeholders::_1));
		value->OnCollisionEnd.Connect(std::bind(&ScriptCCollider::OnCollisionEnd, this, std::placeholders::_1));
	}

	void ScriptCCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetIsTrigger", (void*)&ScriptCCollider::InternalSetIsTrigger);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsTrigger", (void*)&ScriptCCollider::InternalGetIsTrigger);
		metaData.ScriptClass->AddInternalCall("Internal_SetMass", (void*)&ScriptCCollider::InternalSetMass);
		metaData.ScriptClass->AddInternalCall("Internal_GetMass", (void*)&ScriptCCollider::InternalGetMass);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptCCollider::InternalSetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptCCollider::InternalGetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetContactOffset", (void*)&ScriptCCollider::InternalSetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetContactOffset", (void*)&ScriptCCollider::InternalGetContactOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetRestOffset", (void*)&ScriptCCollider::InternalSetRestOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetRestOffset", (void*)&ScriptCCollider::InternalGetRestOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCCollider::InternalSetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCCollider::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_SetCollisionReportMode", (void*)&ScriptCCollider::InternalSetCollisionReportMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetCollisionReportMode", (void*)&ScriptCCollider::InternalGetCollisionReportMode);

		OnCollisionBeginThunk = (OnCollisionBeginThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
	}

	void ScriptCCollider::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionBeginThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCCollider::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionStayThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCCollider::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionEndThunk, GetManagedInstance(), tmpp0);
	}
	void ScriptCCollider::InternalSetIsTrigger(ScriptCColliderBase* thisPtr, bool value)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetIsTrigger(value);
	}

	bool ScriptCCollider::InternalGetIsTrigger(ScriptCColliderBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetIsTrigger();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::InternalSetMass(ScriptCColliderBase* thisPtr, float mass)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetMass(mass);
	}

	float ScriptCCollider::InternalGetMass(ScriptCColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::InternalSetMaterial(ScriptCColliderBase* thisPtr, MonoObject* material)
	{
		ResourceHandle<PhysicsMaterial> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::ToNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<PhysicsMaterial>(scriptmaterial->GetHandle());
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptCCollider::InternalGetMaterial(ScriptCColliderBase* thisPtr)
	{
		ResourceHandle<PhysicsMaterial> tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCCollider::InternalSetContactOffset(ScriptCColliderBase* thisPtr, float value)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetContactOffset(value);
	}

	float ScriptCCollider::InternalGetContactOffset(ScriptCColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::InternalSetRestOffset(ScriptCColliderBase* thisPtr, float value)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetRestOffset(value);
	}

	float ScriptCCollider::InternalGetRestOffset(ScriptCColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetRestOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::InternalSetLayer(ScriptCColliderBase* thisPtr, uint64_t layer)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetLayer(layer);
	}

	uint64_t ScriptCCollider::InternalGetLayer(ScriptCColliderBase* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::InternalSetCollisionReportMode(ScriptCColliderBase* thisPtr, CollisionReportMode mode)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCCollider::InternalGetCollisionReportMode(ScriptCColliderBase* thisPtr)
	{
		CollisionReportMode tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}
}
