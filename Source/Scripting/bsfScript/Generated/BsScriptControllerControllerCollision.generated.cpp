//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptControllerControllerCollision.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "../../../Foundation/bsfCore/Components/BsCCharacterController.h"
#include "BsScriptCCharacterController.generated.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptControllerControllerCollision::ScriptControllerControllerCollision(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptControllerControllerCollision::InitRuntimeData()
	{ }

	MonoObject*ScriptControllerControllerCollision::Box(const __ControllerControllerCollisionInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ControllerControllerCollisionInterop ScriptControllerControllerCollision::Unbox(MonoObject* value)
	{
		return *(__ControllerControllerCollisionInterop*)MonoUtil::Unbox(value);
	}

	ControllerControllerCollision ScriptControllerControllerCollision::FromInterop(const __ControllerControllerCollisionInterop& value)
	{
		ControllerControllerCollision output;
		GameObjectHandle<CCharacterController> tmpController;
		ScriptCCharacterController* scriptController;
		scriptController = ScriptCCharacterController::ToNative(value.Controller);
		if(scriptController != nullptr)
			tmpController = scriptController->GetHandle();
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
		ScriptComponentBase* scriptController = nullptr;
		if(value.Controller)
			scriptController = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(value.Controller));
		if(scriptController != nullptr)
			tmpController = scriptController->GetManagedInstance();
		else
			tmpController = nullptr;
		output.Controller = tmpController;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

}
