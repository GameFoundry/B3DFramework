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
	ScriptBoxCollider::ScriptBoxCollider(MonoObject* managedInstance, const GameObjectHandle<CBoxCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptBoxCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptBoxCollider::InternalSetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptBoxCollider::InternalGetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptBoxCollider::InternalSetCenter);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptBoxCollider::InternalGetCenter);

	}

	void ScriptBoxCollider::InternalSetExtents(ScriptBoxCollider* thisPtr, Vector3* extents)
	{
		thisPtr->GetHandle()->SetExtents(*extents);
	}

	void ScriptBoxCollider::InternalGetExtents(ScriptBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetExtents();

		*__output = tmp__output;
	}

	void ScriptBoxCollider::InternalSetCenter(ScriptBoxCollider* thisPtr, Vector3* center)
	{
		thisPtr->GetHandle()->SetCenter(*center);
	}

	void ScriptBoxCollider::InternalGetCenter(ScriptBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenter();

		*__output = tmp__output;
	}
}
