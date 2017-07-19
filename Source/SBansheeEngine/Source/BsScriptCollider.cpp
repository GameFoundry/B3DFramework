//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptCollider.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsCollider.h"
#include "BsRigidbody.h"
#include "BsScriptRigidbody.h"
#include "BsScriptCollisionData.h"
#include "BsScriptPhysicsQueryHit.h"
#include "BsScriptResourceManager.h"

#include "BsScriptPhysicsMaterial.generated.h"

using namespace std::placeholders;

namespace bs
{
	ScriptCollider::OnCollisionThunkDef ScriptCollider::onCollisionBeginThunk = nullptr;
	ScriptCollider::OnCollisionThunkDef ScriptCollider::onCollisionStayThunk = nullptr;
	ScriptCollider::OnCollisionThunkDef ScriptCollider::onCollisionEndThunk = nullptr;

	ScriptColliderBase::ScriptColliderBase(MonoObject* instance)
		:ScriptObjectBase(instance)
	{ }

	void ScriptColliderBase::initialize(const SPtr<Collider>& collider)
	{
		mCollider = collider;

		MonoObject* instance = getManagedInstance();
		collider->onCollisionBegin.connect(std::bind(&ScriptCollider::onCollisionBegin, instance, _1));
		collider->onCollisionStay.connect(std::bind(&ScriptCollider::onCollisionStay, instance, _1));
		collider->onCollisionEnd.connect(std::bind(&ScriptCollider::onCollisionEnd, instance, _1));
	}

	void ScriptColliderBase::destroyCollider()
	{
		mCollider = nullptr;
	}

	ScriptCollider::ScriptCollider(MonoObject* instance)
		:TScriptCollider(instance, nullptr)
	{ }

	void ScriptCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptCollider::internal_Destroy);
		metaData.scriptClass->addInternalCall("Internal_GetPosition", &ScriptCollider::internal_GetPosition);
		metaData.scriptClass->addInternalCall("Internal_GetRotation", &ScriptCollider::internal_GetRotation);
		metaData.scriptClass->addInternalCall("Internal_SetTransform", &ScriptCollider::internal_SetTransform);
		metaData.scriptClass->addInternalCall("Internal_SetScale", &ScriptCollider::internal_SetScale);
		metaData.scriptClass->addInternalCall("Internal_GetScale", &ScriptCollider::internal_GetScale);
		metaData.scriptClass->addInternalCall("Internal_SetIsTrigger", &ScriptCollider::internal_SetIsTrigger);
		metaData.scriptClass->addInternalCall("Internal_GetIsTrigger", &ScriptCollider::internal_GetIsTrigger);
		metaData.scriptClass->addInternalCall("Internal_SetRigidbody", &ScriptCollider::internal_SetRigidbody);
		metaData.scriptClass->addInternalCall("Internal_GetRigidbody", &ScriptCollider::internal_GetRigidbody);
		metaData.scriptClass->addInternalCall("Internal_SetMass", &ScriptCollider::internal_SetMass);
		metaData.scriptClass->addInternalCall("Internal_GetMass", &ScriptCollider::internal_GetMass);
		metaData.scriptClass->addInternalCall("Internal_SetMaterial", &ScriptCollider::internal_SetMaterial);
		metaData.scriptClass->addInternalCall("Internal_GetMaterial", &ScriptCollider::internal_GetMaterial);
		metaData.scriptClass->addInternalCall("Internal_SetContactOffset", &ScriptCollider::internal_SetContactOffset);
		metaData.scriptClass->addInternalCall("Internal_GetContactOffset", &ScriptCollider::internal_GetContactOffset);
		metaData.scriptClass->addInternalCall("Internal_SetRestOffset", &ScriptCollider::internal_SetRestOffset);
		metaData.scriptClass->addInternalCall("Internal_GetRestOffset", &ScriptCollider::internal_GetRestOffset);
		metaData.scriptClass->addInternalCall("Internal_SetLayer", &ScriptCollider::internal_SetLayer);
		metaData.scriptClass->addInternalCall("Internal_GetLayer", &ScriptCollider::internal_GetLayer);
		metaData.scriptClass->addInternalCall("Internal_SetCollisionReportMode", &ScriptCollider::internal_SetCollisionReportMode);
		metaData.scriptClass->addInternalCall("Internal_GetCollisionReportMode", &ScriptCollider::internal_GetCollisionReportMode);
		metaData.scriptClass->addInternalCall("Internal_RayCast", &ScriptCollider::internal_RayCast);

		onCollisionBeginThunk = (OnCollisionThunkDef)metaData.scriptClass->getMethod("Internal_DoOnCollisionBegin", 1)->getThunk();
		onCollisionStayThunk = (OnCollisionThunkDef)metaData.scriptClass->getMethod("Internal_DoOnCollisionStay", 1)->getThunk();
		onCollisionEndThunk = (OnCollisionThunkDef)metaData.scriptClass->getMethod("Internal_DoOnCollisionEnd", 1)->getThunk();
	}

	void ScriptCollider::onCollisionBegin(MonoObject* instance, const CollisionData& collisionData)
	{
		MonoObject* managedCollisionData = ScriptCollisionDataHelper::box(ScriptCollisionDataHelper::create(collisionData));
		MonoUtil::invokeThunk(onCollisionBeginThunk, instance, managedCollisionData);
	}

	void ScriptCollider::onCollisionStay(MonoObject* instance, const CollisionData& collisionData)
	{
		MonoObject* managedCollisionData = ScriptCollisionDataHelper::box(ScriptCollisionDataHelper::create(collisionData));
		MonoUtil::invokeThunk(onCollisionStayThunk, instance, managedCollisionData);
	}

	void ScriptCollider::onCollisionEnd(MonoObject* instance, const CollisionData& collisionData)
	{
		MonoObject* managedCollisionData = ScriptCollisionDataHelper::box(ScriptCollisionDataHelper::create(collisionData));
		MonoUtil::invokeThunk(onCollisionEndThunk, instance, managedCollisionData);
	}

	void ScriptCollider::internal_Destroy(ScriptColliderBase* thisPtr)
	{
		thisPtr->destroyCollider();
	}

	void ScriptCollider::internal_GetPosition(ScriptColliderBase* thisPtr, Vector3* pos)
	{
		*pos = thisPtr->getCollider()->getPosition();
	}

	void ScriptCollider::internal_GetRotation(ScriptColliderBase* thisPtr, Quaternion* rot)
	{
		*rot = thisPtr->getCollider()->getRotation();
	}

	void ScriptCollider::internal_SetTransform(ScriptColliderBase* thisPtr, Vector3* pos, Quaternion* rot)
	{
		thisPtr->getCollider()->setTransform(*pos, *rot);
	}

	void ScriptCollider::internal_SetScale(ScriptColliderBase* thisPtr, Vector3* scale)
	{
		thisPtr->getCollider()->setScale(*scale);
	}

	void ScriptCollider::internal_GetScale(ScriptColliderBase* thisPtr, Vector3* scale)
	{
		*scale = thisPtr->getCollider()->getScale();
	}

	void ScriptCollider::internal_SetIsTrigger(ScriptColliderBase* thisPtr, bool value)
	{
		thisPtr->getCollider()->setIsTrigger(value);
	}

	bool ScriptCollider::internal_GetIsTrigger(ScriptColliderBase* thisPtr)
	{
		return thisPtr->getCollider()->getIsTrigger();
	}

	void ScriptCollider::internal_SetRigidbody(ScriptColliderBase* thisPtr, ScriptRigidbody* value)
	{
		Rigidbody* rigidbodyPtr = nullptr;
		if (value != nullptr)
			rigidbodyPtr = value->getRigidbody();

		thisPtr->getCollider()->setRigidbody(rigidbodyPtr);
	}

	MonoObject* ScriptCollider::internal_GetRigidbody(ScriptColliderBase* thisPtr)
	{
		Rigidbody* rigidbodyPtr = thisPtr->getCollider()->getRigidbody();
		if (rigidbodyPtr == nullptr)
			return nullptr;

		return (MonoObject*)rigidbodyPtr->_getOwner(PhysicsOwnerType::Script);
	}

	void ScriptCollider::internal_SetMass(ScriptColliderBase* thisPtr, float mass)
	{
		thisPtr->getCollider()->setMass(mass);
	}

	float ScriptCollider::internal_GetMass(ScriptColliderBase* thisPtr)
	{
		return thisPtr->getCollider()->getMass();
	}

	void ScriptCollider::internal_SetMaterial(ScriptColliderBase* thisPtr, ScriptPhysicsMaterial* material)
	{
		HPhysicsMaterial materialHandle;

		if (material != nullptr)
			materialHandle = material->getHandle();

		thisPtr->getCollider()->setMaterial(materialHandle);
	}

	MonoObject* ScriptCollider::internal_GetMaterial(ScriptColliderBase* thisPtr)
	{
		HPhysicsMaterial material = thisPtr->getCollider()->getMaterial();
		if (material == nullptr)
			return nullptr;

		ScriptResourceBase* scriptMaterial = ScriptResourceManager::instance().getScriptResource(material, true);
		return scriptMaterial->getManagedInstance();
	}

	void ScriptCollider::internal_SetContactOffset(ScriptColliderBase* thisPtr, float value)
	{
		thisPtr->getCollider()->setContactOffset(value);
	}

	float ScriptCollider::internal_GetContactOffset(ScriptColliderBase* thisPtr)
	{
		return thisPtr->getCollider()->getContactOffset();
	}

	void ScriptCollider::internal_SetRestOffset(ScriptColliderBase* thisPtr, float value)
	{
		thisPtr->getCollider()->setRestOffset(value);
	}

	float ScriptCollider::internal_GetRestOffset(ScriptColliderBase* thisPtr)
	{
		return thisPtr->getCollider()->getRestOffset();
	}

	void ScriptCollider::internal_SetLayer(ScriptColliderBase* thisPtr, UINT64 layer)
	{
		thisPtr->getCollider()->setLayer(layer);
	}

	UINT64 ScriptCollider::internal_GetLayer(ScriptColliderBase* thisPtr)
	{
		return thisPtr->getCollider()->getLayer();
	}

	void ScriptCollider::internal_SetCollisionReportMode(ScriptColliderBase* thisPtr, CollisionReportMode mode)
	{
		thisPtr->getCollider()->setCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCollider::internal_GetCollisionReportMode(ScriptColliderBase* thisPtr)
	{
		return thisPtr->getCollider()->getCollisionReportMode();
	}

	bool ScriptCollider::internal_RayCast(ScriptColliderBase* thisPtr, Vector3* origin, Vector3* unitDir, MonoObject** hit,
		float maxDist)
	{
		PhysicsQueryHit nativeHit;
		bool wasHit = thisPtr->getCollider()->rayCast(*origin, *unitDir, nativeHit, maxDist);

		if (!wasHit)
			*hit = nullptr;
		else
			*hit = ScriptPhysicsQueryHitHelper::box(ScriptPhysicsQueryHitHelper::create(nativeHit));

		return wasHit;
	}
}