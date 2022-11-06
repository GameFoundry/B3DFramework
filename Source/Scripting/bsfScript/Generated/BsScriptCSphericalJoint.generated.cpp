//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSphericalJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCSphericalJoint.h"
#include "BsScriptLimitConeRange.generated.h"

namespace bs
{
	ScriptCSphericalJoint::ScriptCSphericalJoint(MonoObject* managedInstance, const GameObjectHandle<CSphericalJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCSphericalJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptCSphericalJoint::InternalGetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptCSphericalJoint::InternalSetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptCSphericalJoint::InternalSetFlag);
		metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptCSphericalJoint::InternalHasFlag);

	}

	void ScriptCSphericalJoint::InternalGetLimit(ScriptCSphericalJoint* thisPtr, __LimitConeRangeInterop* __output)
	{
		LimitConeRange tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLimit();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitConeRange::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptCSphericalJoint::InternalSetLimit(ScriptCSphericalJoint* thisPtr, __LimitConeRangeInterop* limit)
	{
		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::FromInterop(*limit);
		thisPtr->GetHandle()->SetLimit(tmplimit);
	}

	void ScriptCSphericalJoint::InternalSetFlag(ScriptCSphericalJoint* thisPtr, SphericalJointFlag flag, bool enabled)
	{
		thisPtr->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptCSphericalJoint::InternalHasFlag(ScriptCSphericalJoint* thisPtr, SphericalJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
