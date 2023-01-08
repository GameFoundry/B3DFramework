//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSliderJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCSliderJoint.h"
#include "BsScriptLimitLinearRange.generated.h"

namespace bs
{
	ScriptSliderJoint::ScriptSliderJoint(MonoObject* managedInstance, const GameObjectHandle<CSliderJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptSliderJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptSliderJoint::InternalGetPosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptSliderJoint::InternalGetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptSliderJoint::InternalGetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptSliderJoint::InternalSetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptSliderJoint::InternalSetFlag);
		metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptSliderJoint::InternalHasFlag);

	}

	float ScriptSliderJoint::InternalGetPosition(ScriptSliderJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPosition();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptSliderJoint::InternalGetSpeed(ScriptSliderJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSliderJoint::InternalGetLimit(ScriptSliderJoint* thisPtr, __LimitLinearRangeInterop* __output)
	{
		LimitLinearRange tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLimit();

		__LimitLinearRangeInterop interop__output;
		interop__output = ScriptLimitLinearRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitLinearRange::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptSliderJoint::InternalSetLimit(ScriptSliderJoint* thisPtr, __LimitLinearRangeInterop* limit)
	{
		LimitLinearRange tmplimit;
		tmplimit = ScriptLimitLinearRange::FromInterop(*limit);
		thisPtr->GetHandle()->SetLimit(tmplimit);
	}

	void ScriptSliderJoint::InternalSetFlag(ScriptSliderJoint* thisPtr, SliderJointFlag flag, bool enabled)
	{
		thisPtr->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptSliderJoint::InternalHasFlag(ScriptSliderJoint* thisPtr, SliderJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
