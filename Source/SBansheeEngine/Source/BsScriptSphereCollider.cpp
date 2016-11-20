//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSphereCollider.h"
#include "BsSphereCollider.h"

namespace bs
{
	ScriptSphereCollider::ScriptSphereCollider(MonoObject* instance, const SPtr<Collider>& collider)
		:TScriptCollider(instance, collider)
	{

	}

	void ScriptSphereCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptSphereCollider::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_SetRadius", &ScriptSphereCollider::internal_SetRadius);
		metaData.scriptClass->addInternalCall("Internal_GetRadius", &ScriptSphereCollider::internal_GetRadius);
	}

	SphereCollider* ScriptSphereCollider::getSphereCollider() const
	{
		return static_cast<SphereCollider*>(mCollider.get());
	}

	void ScriptSphereCollider::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<SphereCollider> collider = SphereCollider::create(1.0f);
		collider->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptSphereCollider>()) ScriptSphereCollider(instance, collider);
	}

	void ScriptSphereCollider::internal_SetRadius(ScriptSphereCollider* thisPtr, float radius)
	{
		thisPtr->getSphereCollider()->setRadius(radius);
	}

	float ScriptSphereCollider::internal_GetRadius(ScriptSphereCollider* thisPtr)
	{
		return thisPtr->getSphereCollider()->getRadius();
	}
}