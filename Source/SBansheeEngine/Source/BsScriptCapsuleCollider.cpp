//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptCapsuleCollider.h"
#include "BsCapsuleCollider.h"

namespace bs
{
	ScriptCapsuleCollider::ScriptCapsuleCollider(MonoObject* instance, const SPtr<Collider>& collider)
		:TScriptCollider(instance, collider)
	{

	}

	void ScriptCapsuleCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptCapsuleCollider::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_SetHalfHeight", &ScriptCapsuleCollider::internal_SetHalfHeight);
		metaData.scriptClass->addInternalCall("Internal_GetHalfHeight", &ScriptCapsuleCollider::internal_GetHalfHeight);
		metaData.scriptClass->addInternalCall("Internal_SetRadius", &ScriptCapsuleCollider::internal_SetRadius);
		metaData.scriptClass->addInternalCall("Internal_GetRadius", &ScriptCapsuleCollider::internal_GetRadius);
	}

	CapsuleCollider* ScriptCapsuleCollider::getCapsuleCollider() const
	{
		return static_cast<CapsuleCollider*>(mCollider.get());
	}

	void ScriptCapsuleCollider::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<CapsuleCollider> collider = CapsuleCollider::create(0.2f, 0.5f);
		collider->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptCapsuleCollider>()) ScriptCapsuleCollider(instance, collider);
	}

	void ScriptCapsuleCollider::internal_SetHalfHeight(ScriptCapsuleCollider* thisPtr, float halfHeight)
	{
		thisPtr->getCapsuleCollider()->setHalfHeight(halfHeight);
	}

	float ScriptCapsuleCollider::internal_GetHalfHeight(ScriptCapsuleCollider* thisPtr)
	{
		return thisPtr->getCapsuleCollider()->getHalfHeight();
	}

	void ScriptCapsuleCollider::internal_SetRadius(ScriptCapsuleCollider* thisPtr, float radius)
	{
		thisPtr->getCapsuleCollider()->setRadius(radius);
	}

	float ScriptCapsuleCollider::internal_GetRadius(ScriptCapsuleCollider* thisPtr)
	{
		return thisPtr->getCapsuleCollider()->getRadius();
	}
}