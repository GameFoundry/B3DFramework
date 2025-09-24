//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitLinear.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace b3d
{
	ScriptLimitLinear::ScriptLimitLinear()
	{ }

	MonoObject* ScriptLimitLinear::Box(const __LimitLinearInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__LimitLinearInterop ScriptLimitLinear::Unbox(MonoObject* value)
	{
		return *(__LimitLinearInterop*)MonoUtil::Unbox(value);
	}

	LimitLinear ScriptLimitLinear::FromInterop(const __LimitLinearInterop& value)
	{
		LimitLinear output;
		output.Extent = value.Extent;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

	__LimitLinearInterop ScriptLimitLinear::ToInterop(const LimitLinear& value)
	{
		__LimitLinearInterop output;
		output.Extent = value.Extent;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

}
