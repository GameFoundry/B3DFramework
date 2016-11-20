//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptControllerCollision.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsCollider.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptControllerCollisionHelper::ScriptControllerCollisionHelper(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptControllerCollisionHelper::initRuntimeData()
	{ }

	ScriptControllerCollision ScriptControllerCollisionHelper::create(const ControllerColliderCollision& data)
	{
		ScriptControllerCollision output;

		if (data.colliderRaw != nullptr)
			output.collider = (MonoObject*)data.colliderRaw->_getOwner(PhysicsOwnerType::Script);

		output.motionAmount = data.motionAmount;
		output.motionDir = data.motionDir;
		output.normal = data.normal;
		output.position = data.position;
		output.triangleIndex = data.triangleIndex;

		return output;
	}

	ScriptControllerCollision ScriptControllerCollisionHelper::create(const ControllerControllerCollision& data)
	{
		ScriptControllerCollision output;

		if (data.controllerRaw != nullptr)
			output.controller = (MonoObject*)data.controllerRaw->_getOwner(PhysicsOwnerType::Script);

		output.motionAmount = data.motionAmount;
		output.motionDir = data.motionDir;
		output.normal = data.normal;
		output.position = data.position;
		output.triangleIndex = 0;

		return output;
	}

	MonoObject* ScriptControllerCollisionHelper::box(const ScriptControllerCollision& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	ScriptControllerCollision ScriptControllerCollisionHelper::unbox(MonoObject* obj)
	{
		return *(ScriptControllerCollision*)MonoUtil::unbox(obj);
	}
}