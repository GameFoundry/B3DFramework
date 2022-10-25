//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitConeRange.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptLimitConeRange::ScriptLimitConeRange(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptLimitConeRange::InitRuntimeData()
	{}

	MonoObject* ScriptLimitConeRange::Box(const __LimitConeRangeInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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

} // namespace bs
