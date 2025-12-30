//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPlaneCollider.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DPlaneCollider.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptPlaneCollider::ScriptPlaneCollider(const TGameObjectHandle<PlaneCollider>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptPlaneCollider::~ScriptPlaneCollider()
	{
		UnregisterEvents();
	}

	void ScriptPlaneCollider::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNormal", (void*)&ScriptPlaneCollider::InternalSetNormal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNormal", (void*)&ScriptPlaneCollider::InternalGetNormal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDistance", (void*)&ScriptPlaneCollider::InternalSetDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDistance", (void*)&ScriptPlaneCollider::InternalGetDistance);

	}

	MonoObject* ScriptPlaneCollider::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptPlaneCollider::InternalSetNormal(ScriptPlaneCollider* self, TVector3<float>* normal)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PlaneCollider*>(self->GetNativeObject())->SetNormal(*normal);
	}

	void ScriptPlaneCollider::InternalGetNormal(ScriptPlaneCollider* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<PlaneCollider*>(self->GetNativeObject())->GetNormal();

		*__output = tmp__output;
	}

	void ScriptPlaneCollider::InternalSetDistance(ScriptPlaneCollider* self, float distance)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PlaneCollider*>(self->GetNativeObject())->SetDistance(distance);
	}

	float ScriptPlaneCollider::InternalGetDistance(ScriptPlaneCollider* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PlaneCollider*>(self->GetNativeObject())->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
