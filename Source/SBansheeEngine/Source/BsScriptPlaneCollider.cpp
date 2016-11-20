//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPlaneCollider.h"
#include "BsPlaneCollider.h"

namespace bs
{
	ScriptPlaneCollider::ScriptPlaneCollider(MonoObject* instance, const SPtr<Collider>& collider)
		:TScriptCollider(instance, collider)
	{

	}

	void ScriptPlaneCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptPlaneCollider::internal_CreateInstance);
	}

	PlaneCollider* ScriptPlaneCollider::getPlaneCollider() const
	{
		return static_cast<PlaneCollider*>(mCollider.get());
	}

	void ScriptPlaneCollider::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<PlaneCollider> collider = PlaneCollider::create();
		collider->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptPlaneCollider>()) ScriptPlaneCollider(instance, collider);
	}
}