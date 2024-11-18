//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCPlaneCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCPlaneCollider.h"
#include "BsScriptTVector3.generated.h"

namespace bs
{
	ScriptPlaneCollider::ScriptPlaneCollider(const GameObjectHandle<CPlaneCollider>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
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

		static_cast<CPlaneCollider*>(self->GetNativeObject())->SetNormal(*normal);
	}

	void ScriptPlaneCollider::InternalGetNormal(ScriptPlaneCollider* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		TVector3<float> tmp__output;
		tmp__output = static_cast<CPlaneCollider*>(self->GetNativeObject())->GetNormal();

		*__output = tmp__output;
	}

	void ScriptPlaneCollider::InternalSetDistance(ScriptPlaneCollider* self, float distance)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CPlaneCollider*>(self->GetNativeObject())->SetDistance(distance);
	}

	float ScriptPlaneCollider::InternalGetDistance(ScriptPlaneCollider* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CPlaneCollider*>(self->GetNativeObject())->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
