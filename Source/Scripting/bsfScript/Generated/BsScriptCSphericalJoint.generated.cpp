//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptCSphericalJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getLimit", (void*)&ScriptCSphericalJoint::Internal_getLimit);
		metaData.scriptClass->addInternalCall("Internal_setLimit", (void*)&ScriptCSphericalJoint::Internal_setLimit);
		metaData.scriptClass->AddInternalCall("Internal_setFlag", (void*)&ScriptCSphericalJoint::InternalSetFlag);
		metaData.scriptClass->AddInternalCall("Internal_hasFlag", (void*)&ScriptCSphericalJoint::InternalHasFlag);

	}

	void ScriptCSphericalJoint::InternalGetLimit(ScriptCSphericalJoint* thisPtr, __LimitConeRangeInterop* __output)
	{
		LimitConeRange tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLimit();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitConeRange::GetMetaData()->scriptClass->GetInternalClassInternal());
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
