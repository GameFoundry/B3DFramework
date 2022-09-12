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

	MonoObject*ScriptControllerControllerCollision::box(const __ControllerControllerCollisionInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__ControllerControllerCollisionInterop ScriptControllerControllerCollision::Unbox(MonoObject* value)
	{
		return *(__ControllerControllerCollisionInterop*)MonoUtil::unbox(value);
	}

	ControllerControllerCollision ScriptControllerControllerCollision::FromInterop(const __ControllerControllerCollisionInterop& value)
	{
		ControllerControllerCollision output;
		GameObjectHandle<CCharacterController> tmpcontroller;
		ScriptCCharacterController* scriptcontroller;
		scriptcontroller = ScriptCCharacterController::toNative(value.controller);
		if(scriptcontroller != nullptr)
			tmpcontroller = scriptcontroller->GetHandle();
		output.controller = tmpcontroller;
		output.position = value.position;
		output.normal = value.normal;
		output.motionDir = value.motionDir;
		output.motionAmount = value.motionAmount;

		return output;
	}

	__ControllerControllerCollisionInterop ScriptControllerControllerCollision::ToInterop(const ControllerControllerCollision& value)
	{
		__ControllerControllerCollisionInterop output;
		ScriptComponentBase* scriptcontroller = nullptr;
		if(value.controller)
			scriptcontroller = ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(value.controller));
		MonoObject* tmpcontroller;
		if(scriptcontroller != nullptr)
			tmpcontroller = scriptcontroller->GetManagedInstance();
		else
			tmpcontroller = nullptr;
		output.controller = tmpcontroller;
		output.position = value.position;
		output.normal = value.normal;
		output.motionDir = value.motionDir;
		output.motionAmount = value.motionAmount;

		return output;
	}

}
