//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptCollisionData.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsCollider.h"

namespace bs
{
	ScriptCollisionDataHelper::ScriptCollisionDataHelper(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptCollisionDataHelper::initRuntimeData()
	{ }

	ScriptCollisionData ScriptCollisionDataHelper::create(const CollisionData& data)
	{
		ScriptCollisionData output;

		if (data.collidersRaw[0] != nullptr)
			output.colliderA = (MonoObject*)data.collidersRaw[0]->_getOwner(PhysicsOwnerType::Script);
		else
			output.colliderA = nullptr;

		if (data.collidersRaw[1] != nullptr)
			output.colliderB = (MonoObject*)data.collidersRaw[1]->_getOwner(PhysicsOwnerType::Script);
		else
			output.colliderB = nullptr;

		UINT32 numContactPoints = (UINT32)data.contactPoints.size();
		ScriptArray contactPoints = ScriptArray::create<ScriptContactPointHelper>(numContactPoints);
		for (UINT32 i = 0; i < numContactPoints; i++)
			contactPoints.set(i, ScriptContactPointHelper::box(data.contactPoints[i]));

		output.contactPoints = contactPoints.getInternal();
		return output;
	}

	MonoObject* ScriptCollisionDataHelper::box(const ScriptCollisionData& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	ScriptCollisionData ScriptCollisionDataHelper::unbox(MonoObject* obj)
	{
		return *(ScriptCollisionData*)MonoUtil::unbox(obj);
	}

	ScriptContactPointHelper::ScriptContactPointHelper(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptContactPointHelper::initRuntimeData()
	{ }

	MonoObject* ScriptContactPointHelper::box(const ContactPoint& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	ContactPoint ScriptContactPointHelper::unbox(MonoObject* obj)
	{
		return *(ContactPoint*)MonoUtil::unbox(obj);
	}
}