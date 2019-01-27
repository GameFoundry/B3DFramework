//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCPlaneCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCPlaneCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCPlaneCollider::ScriptCPlaneCollider(MonoObject* managedInstance, const GameObjectHandle<CPlaneCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCPlaneCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setNormal", (void*)&ScriptCPlaneCollider::Internal_setNormal);
		metaData.scriptClass->addInternalCall("Internal_getNormal", (void*)&ScriptCPlaneCollider::Internal_getNormal);
		metaData.scriptClass->addInternalCall("Internal_setDistance", (void*)&ScriptCPlaneCollider::Internal_setDistance);
		metaData.scriptClass->addInternalCall("Internal_getDistance", (void*)&ScriptCPlaneCollider::Internal_getDistance);

	}

	void ScriptCPlaneCollider::Internal_setNormal(ScriptCPlaneCollider* thisPtr, Vector3* normal)
	{
		thisPtr->getHandle()->setNormal(*normal);
	}

	void ScriptCPlaneCollider::Internal_getNormal(ScriptCPlaneCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getNormal();

		*__output = tmp__output;
	}

	void ScriptCPlaneCollider::Internal_setDistance(ScriptCPlaneCollider* thisPtr, float distance)
	{
		thisPtr->getHandle()->setDistance(distance);
	}

	float ScriptCPlaneCollider::Internal_getDistance(ScriptCPlaneCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
