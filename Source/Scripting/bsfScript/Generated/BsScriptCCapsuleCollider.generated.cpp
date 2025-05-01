//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCapsuleCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCapsuleCollider.h"
#include "BsScriptTVector3.generated.h"

namespace bs
{
	ScriptCapsuleCollider::ScriptCapsuleCollider(const GameObjectHandle<CCapsuleCollider>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptCapsuleCollider::~ScriptCapsuleCollider()
	{
		UnregisterEvents();
	}

	void ScriptCapsuleCollider::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNormal", (void*)&ScriptCapsuleCollider::InternalSetNormal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNormal", (void*)&ScriptCapsuleCollider::InternalGetNormal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptCapsuleCollider::InternalSetCenter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptCapsuleCollider::InternalGetCenter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHalfHeight", (void*)&ScriptCapsuleCollider::InternalSetHalfHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHalfHeight", (void*)&ScriptCapsuleCollider::InternalGetHalfHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptCapsuleCollider::InternalSetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptCapsuleCollider::InternalGetRadius);

	}

	MonoObject* ScriptCapsuleCollider::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptCapsuleCollider::InternalSetNormal(ScriptCapsuleCollider* self, TVector3<float>* normal)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCapsuleCollider*>(self->GetNativeObject())->SetNormal(*normal);
	}

	void ScriptCapsuleCollider::InternalGetNormal(ScriptCapsuleCollider* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<CCapsuleCollider*>(self->GetNativeObject())->GetNormal();

		*__output = tmp__output;
	}

	void ScriptCapsuleCollider::InternalSetCenter(ScriptCapsuleCollider* self, TVector3<float>* center)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCapsuleCollider*>(self->GetNativeObject())->SetCenter(*center);
	}

	void ScriptCapsuleCollider::InternalGetCenter(ScriptCapsuleCollider* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<CCapsuleCollider*>(self->GetNativeObject())->GetCenter();

		*__output = tmp__output;
	}

	void ScriptCapsuleCollider::InternalSetHalfHeight(ScriptCapsuleCollider* self, float halfHeight)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCapsuleCollider*>(self->GetNativeObject())->SetHalfHeight(halfHeight);
	}

	float ScriptCapsuleCollider::InternalGetHalfHeight(ScriptCapsuleCollider* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCapsuleCollider*>(self->GetNativeObject())->GetHalfHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCapsuleCollider::InternalSetRadius(ScriptCapsuleCollider* self, float radius)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CCapsuleCollider*>(self->GetNativeObject())->SetRadius(radius);
	}

	float ScriptCapsuleCollider::InternalGetRadius(ScriptCapsuleCollider* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CCapsuleCollider*>(self->GetNativeObject())->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
