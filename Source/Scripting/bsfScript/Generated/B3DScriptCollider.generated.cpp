//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptCollider.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DCollider.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptCollisionData.generated.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsMaterial.h"

namespace b3d
{
	ScriptColliderWrapperBase::OnCollisionBeginThunkDefinition ScriptColliderWrapperBase::OnCollisionBeginThunk; 
	ScriptColliderWrapperBase::OnCollisionStayThunkDefinition ScriptColliderWrapperBase::OnCollisionStayThunk; 
	ScriptColliderWrapperBase::OnCollisionEndThunkDefinition ScriptColliderWrapperBase::OnCollisionEndThunk; 

	void ScriptColliderWrapperBase::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionBeginThunk, GetScriptObject(), tmpp0);
	}

	void ScriptColliderWrapperBase::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionStayThunk, GetScriptObject(), tmpp0);
	}

	void ScriptColliderWrapperBase::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionEndThunk, GetScriptObject(), tmpp0);
	}

	void ScriptColliderWrapperBase::RegisterEvents()
	{
		OnCollisionBeginConnection = static_cast<Collider*>(GetNativeObject())->OnCollisionBegin.Connect(std::bind(&ScriptColliderWrapperBase::OnCollisionBegin, this, std::placeholders::_1));
		OnCollisionStayConnection = static_cast<Collider*>(GetNativeObject())->OnCollisionStay.Connect(std::bind(&ScriptColliderWrapperBase::OnCollisionStay, this, std::placeholders::_1));
		OnCollisionEndConnection = static_cast<Collider*>(GetNativeObject())->OnCollisionEnd.Connect(std::bind(&ScriptColliderWrapperBase::OnCollisionEnd, this, std::placeholders::_1));
		ScriptGameObjectWrapper::RegisterEvents();
	}
	void ScriptColliderWrapperBase::UnregisterEvents()
	{
		OnCollisionBeginConnection.Disconnect();
		OnCollisionStayConnection.Disconnect();
		OnCollisionEndConnection.Disconnect();
		ScriptGameObjectWrapper::UnregisterEvents();
	}
	ScriptCollider::ScriptCollider(const TGameObjectHandle<Collider>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptCollider::~ScriptCollider()
	{
		UnregisterEvents();
	}

	void ScriptCollider::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsTrigger", (void*)&ScriptCollider::InternalSetIsTrigger);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIsTrigger", (void*)&ScriptCollider::InternalGetIsTrigger);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMass", (void*)&ScriptCollider::InternalSetMass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMass", (void*)&ScriptCollider::InternalGetMass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptCollider::InternalSetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptCollider::InternalGetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetContactOffset", (void*)&ScriptCollider::InternalSetContactOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetContactOffset", (void*)&ScriptCollider::InternalGetContactOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRestOffset", (void*)&ScriptCollider::InternalSetRestOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRestOffset", (void*)&ScriptCollider::InternalGetRestOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCollider::InternalSetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCollider::InternalGetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCollisionReportMode", (void*)&ScriptCollider::InternalSetCollisionReportMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCollisionReportMode", (void*)&ScriptCollider::InternalGetCollisionReportMode);

		OnCollisionBeginThunk = (OnCollisionBeginThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
	}

	MonoObject* ScriptCollider::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptCollider::InternalSetIsTrigger(ScriptColliderWrapperBase* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Collider*>(self->GetNativeObject())->SetIsTrigger(value);
	}

	bool ScriptCollider::InternalGetIsTrigger(ScriptColliderWrapperBase* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Collider*>(self->GetNativeObject())->GetIsTrigger();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetMass(ScriptColliderWrapperBase* self, float mass)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Collider*>(self->GetNativeObject())->SetMass(mass);
	}

	float ScriptCollider::InternalGetMass(ScriptColliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Collider*>(self->GetNativeObject())->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetMaterial(ScriptColliderWrapperBase* self, MonoObject* material)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<PhysicsMaterial> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::GetScriptObjectWrapper(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<PhysicsMaterial>(scriptObjectWrappermaterial->GetNativeObject());
		static_cast<Collider*>(self->GetNativeObject())->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptCollider::InternalGetMaterial(ScriptColliderWrapperBase* self)
	{
		TResourceHandle<PhysicsMaterial> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Collider*>(self->GetNativeObject())->GetMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCollider::InternalSetContactOffset(ScriptColliderWrapperBase* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Collider*>(self->GetNativeObject())->SetContactOffset(value);
	}

	float ScriptCollider::InternalGetContactOffset(ScriptColliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Collider*>(self->GetNativeObject())->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetRestOffset(ScriptColliderWrapperBase* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Collider*>(self->GetNativeObject())->SetRestOffset(value);
	}

	float ScriptCollider::InternalGetRestOffset(ScriptColliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Collider*>(self->GetNativeObject())->GetRestOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetLayer(ScriptColliderWrapperBase* self, uint64_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Collider*>(self->GetNativeObject())->SetLayer(layer);
	}

	uint64_t ScriptCollider::InternalGetLayer(ScriptColliderWrapperBase* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Collider*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCollider::InternalSetCollisionReportMode(ScriptColliderWrapperBase* self, CollisionReportMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Collider*>(self->GetNativeObject())->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCollider::InternalGetCollisionReportMode(ScriptColliderWrapperBase* self)
	{
		CollisionReportMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Collider*>(self->GetNativeObject())->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}
}
