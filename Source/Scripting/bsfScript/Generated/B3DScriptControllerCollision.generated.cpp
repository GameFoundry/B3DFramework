//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptControllerCollision.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

namespace b3d
{
	ScriptControllerCollision::ScriptControllerCollision()
	{ }

	MonoObject* ScriptControllerCollision::Box(const __ControllerCollisionInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ControllerCollisionInterop ScriptControllerCollision::Unbox(MonoObject* value)
	{
		return *(__ControllerCollisionInterop*)MonoUtil::Unbox(value);
	}

	ControllerCollision ScriptControllerCollision::FromInterop(const __ControllerCollisionInterop& value)
	{
		ControllerCollision output;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

	__ControllerCollisionInterop ScriptControllerCollision::ToInterop(const ControllerCollision& value)
	{
		__ControllerCollisionInterop output;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

}
