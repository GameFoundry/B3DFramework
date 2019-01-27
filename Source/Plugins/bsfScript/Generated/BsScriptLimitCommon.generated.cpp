//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitCommon.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptLimitCommon::ScriptLimitCommon(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptLimitCommon::initRuntimeData()
	{ }

	MonoObject*ScriptLimitCommon::box(const __LimitCommonInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__LimitCommonInterop ScriptLimitCommon::unbox(MonoObject* value)
	{
		return *(__LimitCommonInterop*)MonoUtil::unbox(value);
	}

	LimitCommon ScriptLimitCommon::fromInterop(const __LimitCommonInterop& value)
	{
		LimitCommon output;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

	__LimitCommonInterop ScriptLimitCommon::toInterop(const LimitCommon& value)
	{
		__LimitCommonInterop output;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

}
