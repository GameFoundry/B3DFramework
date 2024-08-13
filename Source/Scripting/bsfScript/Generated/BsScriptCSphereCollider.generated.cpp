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
	ScriptSphereCollider::ScriptSphereCollider(MonoObject* managedInstance, const GameObjectHandle<CSphereCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptSphereCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptSphereCollider::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptSphereCollider::InternalGetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptSphereCollider::InternalSetCenter);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptSphereCollider::InternalGetCenter);

	}

	void ScriptSphereCollider::InternalSetRadius(ScriptSphereCollider* self, float radius)
	{
		self->GetHandle()->SetRadius(radius);
	}

	float ScriptSphereCollider::InternalGetRadius(ScriptSphereCollider* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSphereCollider::InternalSetCenter(ScriptSphereCollider* self, TVector3<float>* center)
	{
		self->GetHandle()->SetCenter(*center);
	}

	void ScriptSphereCollider::InternalGetCenter(ScriptSphereCollider* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetCenter();

		*__output = tmp__output;
	}
}
