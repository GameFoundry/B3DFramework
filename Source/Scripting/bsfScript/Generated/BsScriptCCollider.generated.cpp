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
	ScriptColliderBase::ScriptColliderBase(MonoObject* managedInstance)
		:ScriptComponentBase(managedInstance)
	 { }

	ScriptCollider::OnCollisionBeginThunkDef ScriptCollider::OnCollisionBeginThunk; 
	ScriptCollider::OnCollisionStayThunkDef ScriptCollider::OnCollisionStayThunk; 
	ScriptCollider::OnCollisionEndThunkDef ScriptCollider::OnCollisionEndThunk; 

	ScriptCollider::ScriptCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->OnCollisionBegin.Connect(std::bind(&ScriptCollider::OnCollisionBegin, this, std::placeholders::_1));
		value->OnCollisionStay.Connect(std::bind(&ScriptCollider::OnCollisionStay, this, std::placeholders::_1));
		value->OnCollisionEnd.Connect(std::bind(&ScriptCollider::OnCollisionEnd, this, std::placeholders::_1));
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

		OnCollisionBeginThunk = (OnCollisionBeginThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
	}

	void ScriptCollider::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionBeginThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCollider::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionStayThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCollider::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionEndThunk, GetManagedInstance(), tmpp0);
	}
	void ScriptCollider::InternalSetIsTrigger(ScriptColliderBase* thisPtr, bool value)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetIsTrigger(value);
	}

	bool ScriptCollider::InternalGetIsTrigger(ScriptColliderBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetIsTrigger();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetMass(ScriptColliderBase* thisPtr, float mass)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetMass(mass);
	}

	float ScriptCollider::InternalGetMass(ScriptColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetMaterial(ScriptColliderBase* thisPtr, MonoObject* material)
	{
		ResourceHandle<PhysicsMaterial> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::ToNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<PhysicsMaterial>(scriptmaterial->GetHandle());
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptCollider::InternalGetMaterial(ScriptColliderBase* thisPtr)
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

	void ScriptCollider::InternalSetContactOffset(ScriptColliderBase* thisPtr, float value)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetContactOffset(value);
	}

	float ScriptCollider::InternalGetContactOffset(ScriptColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetRestOffset(ScriptColliderBase* thisPtr, float value)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetRestOffset(value);
	}

	float ScriptCollider::InternalGetRestOffset(ScriptColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetRestOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetLayer(ScriptColliderBase* thisPtr, uint64_t layer)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetLayer(layer);
	}

	uint64_t ScriptCollider::InternalGetLayer(ScriptColliderBase* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetCollisionReportMode(ScriptColliderBase* thisPtr, CollisionReportMode mode)
	{
		B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCollider::InternalGetCollisionReportMode(ScriptColliderBase* thisPtr)
	{
		CollisionReportMode tmp__output;
		tmp__output = B3DStaticGameObjectCast<CCollider>(thisPtr->GetComponent())->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}
}
