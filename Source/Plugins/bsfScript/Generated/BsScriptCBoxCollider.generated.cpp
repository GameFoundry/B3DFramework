//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBoxCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCBoxCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCBoxCollider::ScriptCBoxCollider(MonoObject* managedInstance, const GameObjectHandle<CBoxCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCBoxCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setExtents", (void*)&ScriptCBoxCollider::Internal_setExtents);
		metaData.scriptClass->addInternalCall("Internal_getExtents", (void*)&ScriptCBoxCollider::Internal_getExtents);
		metaData.scriptClass->addInternalCall("Internal_setCenter", (void*)&ScriptCBoxCollider::Internal_setCenter);
		metaData.scriptClass->addInternalCall("Internal_getCenter", (void*)&ScriptCBoxCollider::Internal_getCenter);

	}

	void ScriptCBoxCollider::Internal_setExtents(ScriptCBoxCollider* thisPtr, Vector3* extents)
	{
		thisPtr->getHandle()->setExtents(*extents);
	}

	void ScriptCBoxCollider::Internal_getExtents(ScriptCBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getExtents();

		*__output = tmp__output;
	}

	void ScriptCBoxCollider::Internal_setCenter(ScriptCBoxCollider* thisPtr, Vector3* center)
	{
		thisPtr->getHandle()->setCenter(*center);
	}

	void ScriptCBoxCollider::Internal_getCenter(ScriptCBoxCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getCenter();

		*__output = tmp__output;
	}
}
