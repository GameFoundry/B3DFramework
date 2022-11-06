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
	ScriptCSphereCollider::ScriptCSphereCollider(MonoObject* managedInstance, const GameObjectHandle<CSphereCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCSphereCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptCSphereCollider::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptCSphereCollider::InternalGetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptCSphereCollider::InternalSetCenter);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptCSphereCollider::InternalGetCenter);

	}

	void ScriptCSphereCollider::InternalSetRadius(ScriptCSphereCollider* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetRadius(radius);
	}

	float ScriptCSphereCollider::InternalGetRadius(ScriptCSphereCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCSphereCollider::InternalSetCenter(ScriptCSphereCollider* thisPtr, Vector3* center)
	{
		thisPtr->GetHandle()->SetCenter(*center);
	}

	void ScriptCSphereCollider::InternalGetCenter(ScriptCSphereCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenter();

		*__output = tmp__output;
	}
}
