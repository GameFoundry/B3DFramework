//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptControllerControllerCollision.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "B3DScriptCharacterController.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptControllerControllerCollision::ScriptControllerControllerCollision()
	{ }

	MonoObject* ScriptControllerControllerCollision::Box(const __ControllerControllerCollisionInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ControllerControllerCollisionInterop ScriptControllerControllerCollision::Unbox(MonoObject* value)
	{
		return *(__ControllerControllerCollisionInterop*)MonoUtil::Unbox(value);
	}

	ControllerControllerCollision ScriptControllerControllerCollision::FromInterop(const __ControllerControllerCollisionInterop& value)
	{
		ControllerControllerCollision output;
		TGameObjectHandle<CharacterController> tmpController;
		ScriptCharacterController* scriptObjectWrapperController;
		scriptObjectWrapperController = ScriptCharacterController::GetScriptObjectWrapper(value.Controller);
		if(scriptObjectWrapperController != nullptr)
			tmpController = B3DStaticGameObjectCast<CharacterController>(scriptObjectWrapperController->GetBaseNativeObjectAsHandle());
		output.Controller = tmpController;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

	__ControllerControllerCollisionInterop ScriptControllerControllerCollision::ToInterop(const ControllerControllerCollision& value)
	{
		__ControllerControllerCollisionInterop output;
		MonoObject* tmpController;
		MonoObject* temptmpController = nullptr;
		if(value.Controller)
			temptmpController = ScriptComponent::GetOrCreateScriptObject(value.Controller);
		tmpController = temptmpController;
		output.Controller = tmpController;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

}
