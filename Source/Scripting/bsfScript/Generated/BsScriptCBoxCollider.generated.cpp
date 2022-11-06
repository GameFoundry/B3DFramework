//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBoxCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCBoxCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCBoxCollider::ScriptCBoxCollider(MonoObject* managedInstance, const GameObjectHandle<CBoxCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCBoxCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptCBoxCollider::InternalSetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptCBoxCollider::InternalGetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptCBoxCollider::InternalSetCenter);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptCBoxCollider::InternalGetCenter);

	}

	void ScriptCBoxCollider::InternalSetExtents(ScriptCBoxCollider* thisPtr, Vector3* extents)
	{
		thisPtr->GetHandle()->SetExtents(*extents);
	}

	void ScriptCBoxCollider::InternalGetExtents(ScriptCBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetExtents();

		*__output = tmp__output;
	}

	void ScriptCBoxCollider::InternalSetCenter(ScriptCBoxCollider* thisPtr, Vector3* center)
	{
		thisPtr->GetHandle()->SetCenter(*center);
	}

	void ScriptCBoxCollider::InternalGetCenter(ScriptCBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenter();

		*__output = tmp__output;
	}
}
