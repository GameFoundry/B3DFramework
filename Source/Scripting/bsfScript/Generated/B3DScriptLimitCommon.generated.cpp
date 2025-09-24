//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitCommon.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace b3d
{
	ScriptLimitCommon::ScriptLimitCommon()
	{ }

	MonoObject* ScriptLimitCommon::Box(const __LimitCommonInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__LimitCommonInterop ScriptLimitCommon::Unbox(MonoObject* value)
	{
		return *(__LimitCommonInterop*)MonoUtil::Unbox(value);
	}

	LimitCommon ScriptLimitCommon::FromInterop(const __LimitCommonInterop& value)
	{
		LimitCommon output;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

	__LimitCommonInterop ScriptLimitCommon::ToInterop(const LimitCommon& value)
	{
		__LimitCommonInterop output;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

}
