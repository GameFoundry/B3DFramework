//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCapsuleCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCCapsuleCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCCapsuleCollider::ScriptCCapsuleCollider(MonoObject* managedInstance, const GameObjectHandle<CCapsuleCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCCapsuleCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setNormal", (void*)&ScriptCCapsuleCollider::Internal_setNormal);
		metaData.scriptClass->addInternalCall("Internal_getNormal", (void*)&ScriptCCapsuleCollider::Internal_getNormal);
		metaData.scriptClass->addInternalCall("Internal_setCenter", (void*)&ScriptCCapsuleCollider::Internal_setCenter);
		metaData.scriptClass->addInternalCall("Internal_getCenter", (void*)&ScriptCCapsuleCollider::Internal_getCenter);
		metaData.scriptClass->addInternalCall("Internal_setHalfHeight", (void*)&ScriptCCapsuleCollider::Internal_setHalfHeight);
		metaData.scriptClass->addInternalCall("Internal_getHalfHeight", (void*)&ScriptCCapsuleCollider::Internal_getHalfHeight);
		metaData.scriptClass->addInternalCall("Internal_setRadius", (void*)&ScriptCCapsuleCollider::Internal_setRadius);
		metaData.scriptClass->addInternalCall("Internal_getRadius", (void*)&ScriptCCapsuleCollider::Internal_getRadius);

	}

	void ScriptCCapsuleCollider::Internal_setNormal(ScriptCCapsuleCollider* thisPtr, Vector3* normal)
	{
		thisPtr->getHandle()->setNormal(*normal);
	}

	void ScriptCCapsuleCollider::Internal_getNormal(ScriptCCapsuleCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getNormal();

		*__output = tmp__output;
	}

	void ScriptCCapsuleCollider::Internal_setCenter(ScriptCCapsuleCollider* thisPtr, Vector3* center)
	{
		thisPtr->getHandle()->setCenter(*center);
	}

	void ScriptCCapsuleCollider::Internal_getCenter(ScriptCCapsuleCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getCenter();

		*__output = tmp__output;
	}

	void ScriptCCapsuleCollider::Internal_setHalfHeight(ScriptCCapsuleCollider* thisPtr, float halfHeight)
	{
		thisPtr->getHandle()->setHalfHeight(halfHeight);
	}

	float ScriptCCapsuleCollider::Internal_getHalfHeight(ScriptCCapsuleCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getHalfHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCapsuleCollider::Internal_setRadius(ScriptCCapsuleCollider* thisPtr, float radius)
	{
		thisPtr->getHandle()->setRadius(radius);
	}

	float ScriptCCapsuleCollider::Internal_getRadius(ScriptCCapsuleCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
