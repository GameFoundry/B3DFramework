//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitLinear.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptLimitLinear::ScriptLimitLinear(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptLimitLinear::InitRuntimeData()
	{ }

	MonoObject*ScriptLimitLinear::box(const __LimitLinearInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__LimitLinearInterop ScriptLimitLinear::Unbox(MonoObject* value)
	{
		return *(__LimitLinearInterop*)MonoUtil::unbox(value);
	}

	LimitLinear ScriptLimitLinear::FromInterop(const __LimitLinearInterop& value)
	{
		LimitLinear output;
		output.extent = value.extent;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

	__LimitLinearInterop ScriptLimitLinear::ToInterop(const LimitLinear& value)
	{
		__LimitLinearInterop output;
		output.extent = value.extent;
		output.contactDist = value.contactDist;
		output.restitution = value.restitution;
		output.spring = value.spring;

		return output;
	}

}
