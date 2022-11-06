//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	MonoObject*ScriptLimitLinearRange::Box(const __LimitLinearRangeInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__LimitLinearRangeInterop ScriptLimitLinearRange::Unbox(MonoObject* value)
	{
		return *(__LimitLinearRangeInterop*)MonoUtil::Unbox(value);
	}

	LimitLinearRange ScriptLimitLinearRange::FromInterop(const __LimitLinearRangeInterop& value)
	{
		LimitLinearRange output;
		output.Lower = value.Lower;
		output.Upper = value.Upper;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

	__LimitLinearRangeInterop ScriptLimitLinearRange::ToInterop(const LimitLinearRange& value)
	{
		__LimitLinearRangeInterop output;
		output.Lower = value.Lower;
		output.Upper = value.Upper;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

}
