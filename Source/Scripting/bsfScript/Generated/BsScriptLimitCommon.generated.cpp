//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLimitCommon.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptLimitCommon::ScriptLimitCommon(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptLimitCommon::InitRuntimeData()
	{}

	MonoObject* ScriptLimitCommon::Box(const __LimitCommonInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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

} // namespace bs
