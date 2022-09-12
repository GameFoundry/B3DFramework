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

	void ScriptCSphericalJoint::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getLimit", (void*)&ScriptCSphericalJoint::Internal_getLimit);
		metaData.scriptClass->AddInternalCall("Internal_setLimit", (void*)&ScriptCSphericalJoint::Internal_setLimit);
		metaData.scriptClass->AddInternalCall("Internal_setFlag", (void*)&ScriptCSphericalJoint::Internal_setFlag);
		metaData.scriptClass->AddInternalCall("Internal_hasFlag", (void*)&ScriptCSphericalJoint::Internal_hasFlag);

	}

	void ScriptCSphericalJoint::Internal_getLimit(ScriptCSphericalJoint* thisPtr, __LimitConeRangeInterop* __output)
	{
		LimitConeRange tmp__output;
		tmp__output = thisPtr->GetHandle()->getLimit();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitConeRange::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptCSphericalJoint::Internal_setLimit(ScriptCSphericalJoint* thisPtr, __LimitConeRangeInterop* limit)
	{
		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::fromInterop(*limit);
		thisPtr->GetHandle()->setLimit(tmplimit);
	}

	void ScriptCSphericalJoint::Internal_setFlag(ScriptCSphericalJoint* thisPtr, SphericalJointFlag flag, bool enabled)
	{
		thisPtr->GetHandle()->setFlag(flag, enabled);
	}

	bool ScriptCSphericalJoint::Internal_hasFlag(ScriptCSphericalJoint* thisPtr, SphericalJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->hasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
