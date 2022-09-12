//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_setExtents", (void*)&ScriptCBoxCollider::Internal_setExtents);
		metaData.scriptClass->AddInternalCall("Internal_getExtents", (void*)&ScriptCBoxCollider::Internal_getExtents);
		metaData.scriptClass->AddInternalCall("Internal_setCenter", (void*)&ScriptCBoxCollider::Internal_setCenter);
		metaData.scriptClass->AddInternalCall("Internal_getCenter", (void*)&ScriptCBoxCollider::Internal_getCenter);

	}

	void ScriptCBoxCollider::Internal_setExtents(ScriptCBoxCollider* thisPtr, Vector3* extents)
	{
		thisPtr->GetHandle()->setExtents(*extents);
	}

	void ScriptCBoxCollider::Internal_getExtents(ScriptCBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getExtents();

		*__output = tmp__output;
	}

	void ScriptCBoxCollider::Internal_setCenter(ScriptCBoxCollider* thisPtr, Vector3* center)
	{
		thisPtr->GetHandle()->setCenter(*center);
	}

	void ScriptCBoxCollider::Internal_getCenter(ScriptCBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getCenter();

		*__output = tmp__output;
	}
}
