//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptLimitAngularRange.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "B3DScriptSpring.generated.h"

namespace b3d
{
	ScriptLimitAngularRange::ScriptLimitAngularRange()
	{ }

	MonoObject* ScriptLimitAngularRange::Box(const __LimitAngularRangeInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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

}
