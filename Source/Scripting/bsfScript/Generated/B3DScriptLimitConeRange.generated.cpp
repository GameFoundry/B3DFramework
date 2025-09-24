//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptLimitConeRange.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "B3DScriptSpring.generated.h"

namespace b3d
{
	ScriptLimitConeRange::ScriptLimitConeRange()
	{ }

	MonoObject* ScriptLimitConeRange::Box(const __LimitConeRangeInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__LimitConeRangeInterop ScriptLimitConeRange::Unbox(MonoObject* value)
	{
		return *(__LimitConeRangeInterop*)MonoUtil::Unbox(value);
	}

	LimitConeRange ScriptLimitConeRange::FromInterop(const __LimitConeRangeInterop& value)
	{
		LimitConeRange output;
		output.YLimitAngle = value.YLimitAngle;
		output.ZLimitAngle = value.ZLimitAngle;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

	__LimitConeRangeInterop ScriptLimitConeRange::ToInterop(const LimitConeRange& value)
	{
		__LimitConeRangeInterop output;
		output.YLimitAngle = value.YLimitAngle;
		output.ZLimitAngle = value.ZLimitAngle;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

}
