//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSphereCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCSphereCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCSphereCollider::ScriptCSphereCollider(MonoObject* managedInstance, const GameObjectHandle<CSphereCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCSphereCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setRadius", (void*)&ScriptCSphereCollider::Internal_setRadius);
		metaData.scriptClass->addInternalCall("Internal_getRadius", (void*)&ScriptCSphereCollider::Internal_getRadius);
		metaData.scriptClass->addInternalCall("Internal_setCenter", (void*)&ScriptCSphereCollider::Internal_setCenter);
		metaData.scriptClass->addInternalCall("Internal_getCenter", (void*)&ScriptCSphereCollider::Internal_getCenter);

	}

	void ScriptCSphereCollider::Internal_setRadius(ScriptCSphereCollider* thisPtr, float radius)
	{
		thisPtr->getHandle()->setRadius(radius);
	}

	float ScriptCSphereCollider::Internal_getRadius(ScriptCSphereCollider* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCSphereCollider::Internal_setCenter(ScriptCSphereCollider* thisPtr, Vector3* center)
	{
		thisPtr->getHandle()->setCenter(*center);
	}

	void ScriptCSphereCollider::Internal_getCenter(ScriptCSphereCollider* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getCenter();

		*__output = tmp__output;
	}
}
