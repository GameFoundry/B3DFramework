//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitAngularRange.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptLimitAngularRange::ScriptLimitAngularRange(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptLimitAngularRange::InitRuntimeData()
	{}

	MonoObject* ScriptLimitAngularRange::Box(const __LimitAngularRangeInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__LimitAngularRangeInterop ScriptLimitAngularRange::Unbox(MonoObject* value)
	{
		return *(__LimitAngularRangeInterop*)MonoUtil::Unbox(value);
	}

	LimitAngularRange ScriptLimitAngularRange::FromInterop(const __LimitAngularRangeInterop& value)
	{
		LimitAngularRange output;
		output.Lower = value.Lower;
		output.Upper = value.Upper;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

	__LimitAngularRangeInterop ScriptLimitAngularRange::ToInterop(const LimitAngularRange& value)
	{
		__LimitAngularRangeInterop output;
		output.Lower = value.Lower;
		output.Upper = value.Upper;
		output.ContactDist = value.ContactDist;
		output.Restitution = value.Restitution;
		output.Spring = value.Spring;

		return output;
	}

} // namespace bs
