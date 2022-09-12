//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitLinearRange.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptLimitLinearRange::ScriptLimitLinearRange(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptLimitLinearRange::InitRuntimeData()
	{ }

	MonoObject*ScriptLimitLinearRange::box(const __LimitLinearRangeInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__LimitLinearRangeInterop ScriptLimitLinearRange::Unbox(MonoObject* value)
	{
		return *(__LimitLinearRangeInterop*)MonoUtil::unbox(value);
	}

	LimitLinearRange ScriptLimitLinearRange::FromInterop(const __LimitLinearRangeInterop& value)
	{
		LimitLinearRange output;
		output.lower = value.lower;
		output.upper = value.upper;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

	__LimitLinearRangeInterop ScriptLimitLinearRange::ToInterop(const LimitLinearRange& value)
	{
		__LimitLinearRangeInterop output;
		output.lower = value.lower;
		output.upper = value.upper;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

}
