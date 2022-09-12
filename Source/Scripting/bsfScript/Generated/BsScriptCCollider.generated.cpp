//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	ScriptCCollider::onCollisionBeginThunkDef ScriptCCollider::onCollisionBeginThunk; 
	ScriptCCollider::onCollisionStayThunkDef ScriptCCollider::onCollisionStayThunk; 
	ScriptCCollider::onCollisionEndThunkDef ScriptCCollider::onCollisionEndThunk; 

	ScriptCCollider::ScriptCCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->onCollisionBegin.Connect(std::bind(&ScriptCCollider::onCollisionBegin, this, std::placeholders::_1));
		value->onCollisionStay.Connect(std::bind(&ScriptCCollider::onCollisionStay, this, std::placeholders::_1));
		value->onCollisionEnd.Connect(std::bind(&ScriptCCollider::onCollisionEnd, this, std::placeholders::_1));
	}

	void ScriptCCollider::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setIsTrigger", (void*)&ScriptCCollider::Internal_setIsTrigger);
		metaData.scriptClass->AddInternalCall("Internal_getIsTrigger", (void*)&ScriptCCollider::Internal_getIsTrigger);
		metaData.scriptClass->AddInternalCall("Internal_setMass", (void*)&ScriptCCollider::Internal_setMass);
		metaData.scriptClass->AddInternalCall("Internal_getMass", (void*)&ScriptCCollider::Internal_getMass);
		metaData.scriptClass->AddInternalCall("Internal_setMaterial", (void*)&ScriptCCollider::Internal_setMaterial);
		metaData.scriptClass->AddInternalCall("Internal_getMaterial", (void*)&ScriptCCollider::Internal_getMaterial);
		metaData.scriptClass->AddInternalCall("Internal_setContactOffset", (void*)&ScriptCCollider::Internal_setContactOffset);
		metaData.scriptClass->AddInternalCall("Internal_getContactOffset", (void*)&ScriptCCollider::Internal_getContactOffset);
		metaData.scriptClass->AddInternalCall("Internal_setRestOffset", (void*)&ScriptCCollider::Internal_setRestOffset);
		metaData.scriptClass->AddInternalCall("Internal_getRestOffset", (void*)&ScriptCCollider::Internal_getRestOffset);
		metaData.scriptClass->AddInternalCall("Internal_setLayer", (void*)&ScriptCCollider::Internal_setLayer);
		metaData.scriptClass->AddInternalCall("Internal_getLayer", (void*)&ScriptCCollider::Internal_getLayer);
		metaData.scriptClass->AddInternalCall("Internal_setCollisionReportMode", (void*)&ScriptCCollider::Internal_setCollisionReportMode);
		metaData.scriptClass->AddInternalCall("Internal_getCollisionReportMode", (void*)&ScriptCCollider::Internal_getCollisionReportMode);

		onCollisionBeginThunk = (onCollisionBeginThunkDef)metaData.scriptClass->GetMethodExact("Internal_onCollisionBegin", "CollisionData&")->getThunk();
		onCollisionStayThunk = (onCollisionStayThunkDef)metaData.scriptClass->GetMethodExact("Internal_onCollisionStay", "CollisionData&")->getThunk();
		onCollisionEndThunk = (onCollisionEndThunkDef)metaData.scriptClass->GetMethodExact("Internal_onCollisionEnd", "CollisionData&")->getThunk();
	}

	void ScriptCCollider::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionBeginThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCCollider::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionStayThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCCollider::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionEndThunk, getManagedInstance(), tmpp0);
	}
	void ScriptCCollider::Internal_setIsTrigger(ScriptCColliderBase* thisPtr, bool value)
	{
		static_object_cast<CCollider>(thisPtr->GetComponent())->setIsTrigger(value);
	}

	bool ScriptCCollider::Internal_getIsTrigger(ScriptCColliderBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = static_object_cast<CCollider>(thisPtr->GetComponent())->getIsTrigger();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::Internal_setMass(ScriptCColliderBase* thisPtr, float mass)
	{
		static_object_cast<CCollider>(thisPtr->GetComponent())->setMass(mass);
	}

	float ScriptCCollider::Internal_getMass(ScriptCColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_object_cast<CCollider>(thisPtr->GetComponent())->getMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::Internal_setMaterial(ScriptCColliderBase* thisPtr, MonoObject* material)
	{
		ResourceHandle<PhysicsMaterial> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::toNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = static_resource_cast<PhysicsMaterial>(scriptmaterial->GetHandle());
		static_object_cast<CCollider>(thisPtr->GetComponent())->setMaterial(tmpmaterial);
	}

	MonoObject* ScriptCCollider::Internal_getMaterial(ScriptCColliderBase* thisPtr)
	{
		ResourceHandle<PhysicsMaterial> tmp__output;
		tmp__output = static_object_cast<CCollider>(thisPtr->GetComponent())->getMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCCollider::Internal_setContactOffset(ScriptCColliderBase* thisPtr, float value)
	{
		static_object_cast<CCollider>(thisPtr->GetComponent())->setContactOffset(value);
	}

	float ScriptCCollider::Internal_getContactOffset(ScriptCColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_object_cast<CCollider>(thisPtr->GetComponent())->getContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::Internal_setRestOffset(ScriptCColliderBase* thisPtr, float value)
	{
		static_object_cast<CCollider>(thisPtr->GetComponent())->setRestOffset(value);
	}

	float ScriptCCollider::Internal_getRestOffset(ScriptCColliderBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_object_cast<CCollider>(thisPtr->GetComponent())->getRestOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::Internal_setLayer(ScriptCColliderBase* thisPtr, uint64_t layer)
	{
		static_object_cast<CCollider>(thisPtr->GetComponent())->setLayer(layer);
	}

	uint64_t ScriptCCollider::Internal_getLayer(ScriptCColliderBase* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = static_object_cast<CCollider>(thisPtr->GetComponent())->getLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCollider::Internal_setCollisionReportMode(ScriptCColliderBase* thisPtr, CollisionReportMode mode)
	{
		static_object_cast<CCollider>(thisPtr->GetComponent())->setCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCCollider::Internal_getCollisionReportMode(ScriptCColliderBase* thisPtr)
	{
		CollisionReportMode tmp__output;
		tmp__output = static_object_cast<CCollider>(thisPtr->GetComponent())->getCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}
}
