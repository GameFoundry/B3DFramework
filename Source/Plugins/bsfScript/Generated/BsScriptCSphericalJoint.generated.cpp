//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSphericalJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCSphericalJoint.h"
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
		metaData.scriptClass->addInternalCall("Internal_setFlag", (void*)&ScriptCSphericalJoint::Internal_setFlag);
		metaData.scriptClass->addInternalCall("Internal_hasFlag", (void*)&ScriptCSphericalJoint::Internal_hasFlag);

	}

	void ScriptCSphericalJoint::Internal_getLimit(ScriptCSphericalJoint* thisPtr, __LimitConeRangeInterop* __output)
	{
		LimitConeRange tmp__output;
		tmp__output = thisPtr->getHandle()->getLimit();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitConeRange::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptCSphericalJoint::Internal_setLimit(ScriptCSphericalJoint* thisPtr, __LimitConeRangeInterop* limit)
	{
		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::fromInterop(*limit);
		thisPtr->getHandle()->setLimit(tmplimit);
	}

	void ScriptCSphericalJoint::Internal_setFlag(ScriptCSphericalJoint* thisPtr, SphericalJointFlag flag, bool enabled)
	{
		thisPtr->getHandle()->setFlag(flag, enabled);
	}

	bool ScriptCSphericalJoint::Internal_hasFlag(ScriptCSphericalJoint* thisPtr, SphericalJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->hasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
