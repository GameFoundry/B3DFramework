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

	MonoObject*ScriptD6JointDrive::Box(const __D6JointDriveInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__D6JointDriveInterop ScriptD6JointDrive::Unbox(MonoObject* value)
	{
		return *(__D6JointDriveInterop*)MonoUtil::Unbox(value);
	}

	D6JointDrive ScriptD6JointDrive::FromInterop(const __D6JointDriveInterop& value)
	{
		D6JointDrive output;
		output.Stiffness = value.Stiffness;
		output.Damping = value.Damping;
		output.ForceLimit = value.ForceLimit;
		output.Acceleration = value.Acceleration;

		return output;
	}

	__D6JointDriveInterop ScriptD6JointDrive::ToInterop(const D6JointDrive& value)
	{
		__D6JointDriveInterop output;
		output.Stiffness = value.Stiffness;
		output.Damping = value.Damping;
		output.ForceLimit = value.ForceLimit;
		output.Acceleration = value.Acceleration;

		return output;
	}

}
