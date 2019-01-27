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
		:ScriptObject(managedInstance)
	{ }

	void ScriptControllerCollision::initRuntimeData()
	{ }

	MonoObject*ScriptControllerCollision::box(const __ControllerCollisionInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__ControllerCollisionInterop ScriptControllerCollision::unbox(MonoObject* value)
	{
		return *(__ControllerCollisionInterop*)MonoUtil::unbox(value);
	}

	ControllerCollision ScriptControllerCollision::fromInterop(const __ControllerCollisionInterop& value)
	{
		ControllerCollision output;
		output.position = value.position;
		output.normal = value.normal;
		output.motionDir = value.motionDir;
		output.motionAmount = value.motionAmount;

		return output;
	}

	__ControllerCollisionInterop ScriptControllerCollision::toInterop(const ControllerCollision& value)
	{
		__ControllerCollisionInterop output;
		output.position = value.position;
		output.normal = value.normal;
		output.motionDir = value.motionDir;
		output.motionAmount = value.motionAmount;

		return output;
	}

}
