//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCDistanceJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCDistanceJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptCDistanceJoint::ScriptCDistanceJoint(MonoObject* managedInstance, const GameObjectHandle<CDistanceJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCDistanceJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getDistance", (void*)&ScriptCDistanceJoint::Internal_getDistance);
		metaData.scriptClass->addInternalCall("Internal_getMinDistance", (void*)&ScriptCDistanceJoint::Internal_getMinDistance);
		metaData.scriptClass->addInternalCall("Internal_setMinDistance", (void*)&ScriptCDistanceJoint::Internal_setMinDistance);
		metaData.scriptClass->addInternalCall("Internal_getMaxDistance", (void*)&ScriptCDistanceJoint::Internal_getMaxDistance);
		metaData.scriptClass->addInternalCall("Internal_setMaxDistance", (void*)&ScriptCDistanceJoint::Internal_setMaxDistance);
		metaData.scriptClass->addInternalCall("Internal_getTolerance", (void*)&ScriptCDistanceJoint::Internal_getTolerance);
		metaData.scriptClass->addInternalCall("Internal_setTolerance", (void*)&ScriptCDistanceJoint::Internal_setTolerance);
		metaData.scriptClass->addInternalCall("Internal_getSpring", (void*)&ScriptCDistanceJoint::Internal_getSpring);
		metaData.scriptClass->addInternalCall("Internal_setSpring", (void*)&ScriptCDistanceJoint::Internal_setSpring);
		metaData.scriptClass->addInternalCall("Internal_setFlag", (void*)&ScriptCDistanceJoint::Internal_setFlag);
		metaData.scriptClass->addInternalCall("Internal_hasFlag", (void*)&ScriptCDistanceJoint::Internal_hasFlag);

	}

	float ScriptCDistanceJoint::Internal_getDistance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptCDistanceJoint::Internal_getMinDistance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDistanceJoint::Internal_setMinDistance(ScriptCDistanceJoint* thisPtr, float value)
	{
		thisPtr->getHandle()->setMinDistance(value);
	}

	float ScriptCDistanceJoint::Internal_getMaxDistance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDistanceJoint::Internal_setMaxDistance(ScriptCDistanceJoint* thisPtr, float value)
	{
		thisPtr->getHandle()->setMaxDistance(value);
	}

	float ScriptCDistanceJoint::Internal_getTolerance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getTolerance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDistanceJoint::Internal_setTolerance(ScriptCDistanceJoint* thisPtr, float value)
	{
		thisPtr->getHandle()->setTolerance(value);
	}

	void ScriptCDistanceJoint::Internal_getSpring(ScriptCDistanceJoint* thisPtr, Spring* __output)
	{
		Spring tmp__output;
		tmp__output = thisPtr->getHandle()->getSpring();

		*__output = tmp__output;
	}

	void ScriptCDistanceJoint::Internal_setSpring(ScriptCDistanceJoint* thisPtr, Spring* value)
	{
		thisPtr->getHandle()->setSpring(*value);
	}

	void ScriptCDistanceJoint::Internal_setFlag(ScriptCDistanceJoint* thisPtr, DistanceJointFlag flag, bool enabled)
	{
		thisPtr->getHandle()->setFlag(flag, enabled);
	}

	bool ScriptCDistanceJoint::Internal_hasFlag(ScriptCDistanceJoint* thisPtr, DistanceJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->hasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
