//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCPlaneCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCPlaneCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCPlaneCollider::ScriptCPlaneCollider(MonoObject* managedInstance, const GameObjectHandle<CPlaneCollider>& value)
		: TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCPlaneCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetNormal", (void*)&ScriptCPlaneCollider::InternalSetNormal);
		metaData.ScriptClass->AddInternalCall("Internal_GetNormal", (void*)&ScriptCPlaneCollider::InternalGetNormal);
		metaData.ScriptClass->AddInternalCall("Internal_SetDistance", (void*)&ScriptCPlaneCollider::InternalSetDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetDistance", (void*)&ScriptCPlaneCollider::InternalGetDistance);
	}

	void ScriptCPlaneCollider::InternalSetNormal(ScriptCPlaneCollider* thisPtr, Vector3* normal)
	{
		thisPtr->GetHandle()->SetNormal(*normal);
	}

	void ScriptCPlaneCollider::InternalGetNormal(ScriptCPlaneCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNormal();

		*__output = tmp__output;
	}

	void ScriptCPlaneCollider::InternalSetDistance(ScriptCPlaneCollider* thisPtr, float distance)
	{
		thisPtr->GetHandle()->SetDistance(distance);
	}

	float ScriptCPlaneCollider::InternalGetDistance(ScriptCPlaneCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}
} // namespace bs
