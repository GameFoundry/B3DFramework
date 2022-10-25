//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptControllerCollision.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptControllerCollision::ScriptControllerCollision(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptControllerCollision::InitRuntimeData()
	{}

	MonoObject* ScriptControllerCollision::Box(const __ControllerCollisionInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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

} // namespace bs
