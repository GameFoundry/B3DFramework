//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptD6JointDrive.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptD6JointDrive::ScriptD6JointDrive(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptD6JointDrive::InitRuntimeData()
	{ }

	MonoObject*ScriptD6JointDrive::Box(const D6JointDrive& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	D6JointDrive ScriptD6JointDrive::Unbox(MonoObject* value)
	{
		return *(D6JointDrive*)MonoUtil::Unbox(value);
	}

}
