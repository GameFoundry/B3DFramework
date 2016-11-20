//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPhysicsQueryHit.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsCollider.h"

namespace bs
{
	ScriptPhysicsQueryHitHelper::ScriptPhysicsQueryHitHelper(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPhysicsQueryHitHelper::initRuntimeData()
	{ }

	ScriptPhysicsQueryHit ScriptPhysicsQueryHitHelper::create(const PhysicsQueryHit& data)
	{
		ScriptPhysicsQueryHit output;

		if (data.colliderRaw != nullptr)
			output.collider = (MonoObject*)data.colliderRaw->_getOwner(PhysicsOwnerType::Script);
		else
			output.collider = nullptr;

		output.distance = data.distance;
		output.normal = data.normal;
		output.point = data.point;
		output.triangleIdx = data.triangleIdx;
		output.uv = data.uv;

		return output;
	}

	MonoObject* ScriptPhysicsQueryHitHelper::box(const ScriptPhysicsQueryHit& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	ScriptPhysicsQueryHit ScriptPhysicsQueryHitHelper::unbox(MonoObject* obj)
	{
		return *(ScriptPhysicsQueryHit*)MonoUtil::unbox(obj);
	}
}