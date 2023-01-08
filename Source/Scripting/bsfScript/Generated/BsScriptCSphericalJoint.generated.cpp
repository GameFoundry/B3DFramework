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
	ScriptSphericalJoint::ScriptSphericalJoint(MonoObject* managedInstance, const GameObjectHandle<CSphericalJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptSphericalJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptSphericalJoint::InternalGetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptSphericalJoint::InternalSetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptSphericalJoint::InternalSetFlag);
		metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptSphericalJoint::InternalHasFlag);

	}

	void ScriptSphericalJoint::InternalGetLimit(ScriptSphericalJoint* thisPtr, __LimitConeRangeInterop* __output)
	{
		LimitConeRange tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLimit();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitConeRange::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptSphericalJoint::InternalSetLimit(ScriptSphericalJoint* thisPtr, __LimitConeRangeInterop* limit)
	{
		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::FromInterop(*limit);
		thisPtr->GetHandle()->SetLimit(tmplimit);
	}

	void ScriptSphericalJoint::InternalSetFlag(ScriptSphericalJoint* thisPtr, SphericalJointFlag flag, bool enabled)
	{
		thisPtr->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptSphericalJoint::InternalHasFlag(ScriptSphericalJoint* thisPtr, SphericalJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
