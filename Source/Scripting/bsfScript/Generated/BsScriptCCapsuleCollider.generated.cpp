//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCapsuleCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCapsuleCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCapsuleCollider::ScriptCapsuleCollider(MonoObject* managedInstance, const GameObjectHandle<CCapsuleCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCapsuleCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetNormal", (void*)&ScriptCapsuleCollider::InternalSetNormal);
		metaData.ScriptClass->AddInternalCall("Internal_GetNormal", (void*)&ScriptCapsuleCollider::InternalGetNormal);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptCapsuleCollider::InternalSetCenter);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptCapsuleCollider::InternalGetCenter);
		metaData.ScriptClass->AddInternalCall("Internal_SetHalfHeight", (void*)&ScriptCapsuleCollider::InternalSetHalfHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetHalfHeight", (void*)&ScriptCapsuleCollider::InternalGetHalfHeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptCapsuleCollider::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptCapsuleCollider::InternalGetRadius);

	}

	void ScriptCapsuleCollider::InternalSetNormal(ScriptCapsuleCollider* thisPtr, Vector3* normal)
	{
		thisPtr->GetHandle()->SetNormal(*normal);
	}

	void ScriptCapsuleCollider::InternalGetNormal(ScriptCapsuleCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNormal();

		*__output = tmp__output;
	}

	void ScriptCapsuleCollider::InternalSetCenter(ScriptCapsuleCollider* thisPtr, Vector3* center)
	{
		thisPtr->GetHandle()->SetCenter(*center);
	}

	void ScriptCapsuleCollider::InternalGetCenter(ScriptCapsuleCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenter();

		*__output = tmp__output;
	}

	void ScriptCapsuleCollider::InternalSetHalfHeight(ScriptCapsuleCollider* thisPtr, float halfHeight)
	{
		thisPtr->GetHandle()->SetHalfHeight(halfHeight);
	}

	float ScriptCapsuleCollider::InternalGetHalfHeight(ScriptCapsuleCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetHalfHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCapsuleCollider::InternalSetRadius(ScriptCapsuleCollider* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetRadius(radius);
	}

	float ScriptCapsuleCollider::InternalGetRadius(ScriptCapsuleCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
