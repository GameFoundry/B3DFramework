//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSliderJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCSliderJoint.h"
#include "BsScriptLimitLinearRange.generated.h"

namespace bs
{
	ScriptCSliderJoint::ScriptCSliderJoint(MonoObject* managedInstance, const GameObjectHandle<CSliderJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCSliderJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getPosition", (void*)&ScriptCSliderJoint::Internal_getPosition);
		metaData.scriptClass->addInternalCall("Internal_getSpeed", (void*)&ScriptCSliderJoint::Internal_getSpeed);
		metaData.scriptClass->addInternalCall("Internal_getLimit", (void*)&ScriptCSliderJoint::Internal_getLimit);
		metaData.scriptClass->addInternalCall("Internal_setLimit", (void*)&ScriptCSliderJoint::Internal_setLimit);
		metaData.scriptClass->addInternalCall("Internal_setFlag", (void*)&ScriptCSliderJoint::Internal_setFlag);
		metaData.scriptClass->addInternalCall("Internal_hasFlag", (void*)&ScriptCSliderJoint::Internal_hasFlag);

	}

	float ScriptCSliderJoint::Internal_getPosition(ScriptCSliderJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getPosition();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptCSliderJoint::Internal_getSpeed(ScriptCSliderJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCSliderJoint::Internal_getLimit(ScriptCSliderJoint* thisPtr, __LimitLinearRangeInterop* __output)
	{
		LimitLinearRange tmp__output;
		tmp__output = thisPtr->getHandle()->getLimit();

		__LimitLinearRangeInterop interop__output;
		interop__output = ScriptLimitLinearRange::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitLinearRange::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptCSliderJoint::Internal_setLimit(ScriptCSliderJoint* thisPtr, __LimitLinearRangeInterop* limit)
	{
		LimitLinearRange tmplimit;
		tmplimit = ScriptLimitLinearRange::fromInterop(*limit);
		thisPtr->getHandle()->setLimit(tmplimit);
	}

	void ScriptCSliderJoint::Internal_setFlag(ScriptCSliderJoint* thisPtr, SliderJointFlag flag, bool enabled)
	{
		thisPtr->getHandle()->setFlag(flag, enabled);
	}

	bool ScriptCSliderJoint::Internal_hasFlag(ScriptCSliderJoint* thisPtr, SliderJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->hasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
