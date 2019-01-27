//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitConeRange.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptLimitConeRange::ScriptLimitConeRange(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptLimitConeRange::initRuntimeData()
	{ }

	MonoObject*ScriptLimitConeRange::box(const __LimitConeRangeInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__LimitConeRangeInterop ScriptLimitConeRange::unbox(MonoObject* value)
	{
		return *(__LimitConeRangeInterop*)MonoUtil::unbox(value);
	}

	LimitConeRange ScriptLimitConeRange::fromInterop(const __LimitConeRangeInterop& value)
	{
		LimitConeRange output;
		output.yLimitAngle = value.yLimitAngle;
		output.zLimitAngle = value.zLimitAngle;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

	__LimitConeRangeInterop ScriptLimitConeRange::toInterop(const LimitConeRange& value)
	{
		__LimitConeRangeInterop output;
		output.yLimitAngle = value.yLimitAngle;
		output.zLimitAngle = value.zLimitAngle;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

}
