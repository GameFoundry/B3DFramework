//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCHingeJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCHingeJoint.h"
#include "BsScriptHingeJointDrive.generated.h"
#include "BsScriptLimitAngularRange.generated.h"

namespace bs
{
	ScriptCHingeJoint::ScriptCHingeJoint(MonoObject* managedInstance, const GameObjectHandle<CHingeJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCHingeJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getAngle", (void*)&ScriptCHingeJoint::Internal_getAngle);
		metaData.scriptClass->addInternalCall("Internal_getSpeed", (void*)&ScriptCHingeJoint::Internal_getSpeed);
		metaData.scriptClass->addInternalCall("Internal_getLimit", (void*)&ScriptCHingeJoint::Internal_getLimit);
		metaData.scriptClass->addInternalCall("Internal_setLimit", (void*)&ScriptCHingeJoint::Internal_setLimit);
		metaData.scriptClass->addInternalCall("Internal_getDrive", (void*)&ScriptCHingeJoint::Internal_getDrive);
		metaData.scriptClass->addInternalCall("Internal_setDrive", (void*)&ScriptCHingeJoint::Internal_setDrive);
		metaData.scriptClass->addInternalCall("Internal_setFlag", (void*)&ScriptCHingeJoint::Internal_setFlag);
		metaData.scriptClass->addInternalCall("Internal_hasFlag", (void*)&ScriptCHingeJoint::Internal_hasFlag);

	}

	void ScriptCHingeJoint::Internal_getAngle(ScriptCHingeJoint* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->getHandle()->getAngle();

		*__output = tmp__output;
	}

	float ScriptCHingeJoint::Internal_getSpeed(ScriptCHingeJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCHingeJoint::Internal_getLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* __output)
	{
		LimitAngularRange tmp__output;
		tmp__output = thisPtr->getHandle()->getLimit();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitAngularRange::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptCHingeJoint::Internal_setLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* limit)
	{
		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::fromInterop(*limit);
		thisPtr->getHandle()->setLimit(tmplimit);
	}

	void ScriptCHingeJoint::Internal_getDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* __output)
	{
		HingeJointDrive tmp__output;
		tmp__output = thisPtr->getHandle()->getDrive();

		*__output = tmp__output;
	}

	void ScriptCHingeJoint::Internal_setDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* drive)
	{
		thisPtr->getHandle()->setDrive(*drive);
	}

	void ScriptCHingeJoint::Internal_setFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag, bool enabled)
	{
		thisPtr->getHandle()->setFlag(flag, enabled);
	}

	bool ScriptCHingeJoint::Internal_hasFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->hasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
