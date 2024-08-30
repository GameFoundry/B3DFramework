//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSphereCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCSphereCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptSphereCollider::ScriptSphereCollider(const GameObjectHandle<CSphereCollider>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
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
		static_cast<CSphereCollider*>(self->GetNativeObject())->SetRadius(radius);
	}

	float ScriptSphereCollider::InternalGetRadius(ScriptSphereCollider* self)
	{
		float tmp__output;
		tmp__output = static_cast<CSphereCollider*>(self->GetNativeObject())->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSphereCollider::InternalSetCenter(ScriptSphereCollider* self, TVector3<float>* center)
	{
		static_cast<CSphereCollider*>(self->GetNativeObject())->SetCenter(*center);
	}

	void ScriptSphereCollider::InternalGetCenter(ScriptSphereCollider* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CSphereCollider*>(self->GetNativeObject())->GetCenter();

		*__output = tmp__output;
	}
}
