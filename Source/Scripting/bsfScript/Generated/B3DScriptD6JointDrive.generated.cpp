//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptD6JointDrive.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptD6JointDrive::ScriptD6JointDrive()
	{ }

	MonoObject* ScriptD6JointDrive::Box(const __D6JointDriveInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
