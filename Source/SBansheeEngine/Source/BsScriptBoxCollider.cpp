//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptBoxCollider.h"
#include "BsBoxCollider.h"

namespace bs
{
	ScriptBoxCollider::ScriptBoxCollider(MonoObject* instance, const SPtr<Collider>& collider)
		:TScriptCollider(instance, collider)
	{
		
	}

	void ScriptBoxCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptBoxCollider::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_SetExtents", &ScriptBoxCollider::internal_SetExtents);
		metaData.scriptClass->addInternalCall("Internal_GetExtents", &ScriptBoxCollider::internal_GetExtents);
	}

	BoxCollider* ScriptBoxCollider::getBoxCollider() const
	{
		return static_cast<BoxCollider*>(mCollider.get());
	}

	void ScriptBoxCollider::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<BoxCollider> collider = BoxCollider::create(Vector3::ONE);
		collider->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptBoxCollider>()) ScriptBoxCollider(instance, collider);
	}

	void ScriptBoxCollider::internal_SetExtents(ScriptBoxCollider* thisPtr, Vector3* extents)
	{
		thisPtr->getBoxCollider()->setExtents(*extents);
	}

	void ScriptBoxCollider::internal_GetExtents(ScriptBoxCollider* thisPtr, Vector3* extents)
	{
		*extents = thisPtr->getBoxCollider()->getExtents();
	}
}