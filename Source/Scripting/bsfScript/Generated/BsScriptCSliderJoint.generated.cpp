//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSliderJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCSliderJoint.h"
#include "BsScriptLimitLinearRange.generated.h"

namespace bs
{
	ScriptCSliderJoint::ScriptCSliderJoint(MonoObject* managedInstance, const GameObjectHandle<CSliderJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCSliderJoint::initRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getPosition", (void*)&ScriptCSliderJoint::InternalGetPosition);
		metaData.scriptClass->AddInternalCall("Internal_getSpeed", (void*)&ScriptCSliderJoint::InternalGetSpeed);
		metaData.scriptClass->addInternalCall("Internal_getLimit", (void*)&ScriptCSliderJoint::Internal_getLimit);
		metaData.scriptClass->addInternalCall("Internal_setLimit", (void*)&ScriptCSliderJoint::Internal_setLimit);
		metaData.scriptClass->AddInternalCall("Internal_setFlag", (void*)&ScriptCSliderJoint::InternalSetFlag);
		metaData.scriptClass->AddInternalCall("Internal_hasFlag", (void*)&ScriptCSliderJoint::InternalHasFlag);

	}

	float ScriptCSliderJoint::InternalGetPosition(ScriptCSliderJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPosition();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptCSliderJoint::InternalGetSpeed(ScriptCSliderJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCSliderJoint::InternalGetLimit(ScriptCSliderJoint* thisPtr, __LimitLinearRangeInterop* __output)
	{
		LimitLinearRange tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLimit();

		__LimitLinearRangeInterop interop__output;
		interop__output = ScriptLimitLinearRange::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitLinearRange::getMetaData()->scriptClass->GetInternalClassInternal());
	}

	void ScriptCSliderJoint::InternalSetLimit(ScriptCSliderJoint* thisPtr, __LimitLinearRangeInterop* limit)
	{
		LimitLinearRange tmplimit;
		tmplimit = ScriptLimitLinearRange::fromInterop(*limit);
		thisPtr->GetHandle()->SetLimit(tmplimit);
	}

	void ScriptCSliderJoint::InternalSetFlag(ScriptCSliderJoint* thisPtr, SliderJointFlag flag, bool enabled)
	{
		thisPtr->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptCSliderJoint::InternalHasFlag(ScriptCSliderJoint* thisPtr, SliderJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
