//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSphereCollider.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DSphereCollider.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptSphereCollider::ScriptSphereCollider(const TGameObjectHandle<SphereCollider>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSphereCollider::~ScriptSphereCollider()
	{
		UnregisterEvents();
	}

	void ScriptSphereCollider::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptSphereCollider::InternalSetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptSphereCollider::InternalGetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptSphereCollider::InternalSetCenter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptSphereCollider::InternalGetCenter);

	}

	MonoObject* ScriptSphereCollider::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptSphereCollider::InternalSetRadius(ScriptSphereCollider* self, float radius)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SphereCollider*>(self->GetNativeObject())->SetRadius(radius);
	}

	float ScriptSphereCollider::InternalGetRadius(ScriptSphereCollider* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SphereCollider*>(self->GetNativeObject())->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSphereCollider::InternalSetCenter(ScriptSphereCollider* self, TVector3<float>* center)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SphereCollider*>(self->GetNativeObject())->SetCenter(*center);
	}

	void ScriptSphereCollider::InternalGetCenter(ScriptSphereCollider* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<SphereCollider*>(self->GetNativeObject())->GetCenter();

		*__output = tmp__output;
	}
}
