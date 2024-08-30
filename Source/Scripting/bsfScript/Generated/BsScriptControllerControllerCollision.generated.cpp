//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptControllerControllerCollision.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
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
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ControllerControllerCollisionInterop ScriptControllerControllerCollision::Unbox(MonoObject* value)
	{
		return *(__ControllerControllerCollisionInterop*)MonoUtil::Unbox(value);
	}

	ControllerControllerCollision ScriptControllerControllerCollision::FromInterop(const __ControllerControllerCollisionInterop& value)
	{
		ControllerControllerCollision output;
		GameObjectHandle<CCharacterController> tmpController;
		ScriptCharacterController* scriptWrapperObjectController;
		scriptWrapperObjectController = ScriptCharacterController::GetScriptObjectWrapper(value.Controller);
		if(scriptWrapperObjectController != nullptr)
			tmpController = B3DStaticGameObjectCast<CCharacterController>(scriptWrapperObjectController->GetBaseNativeObjectAsHandle());
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
