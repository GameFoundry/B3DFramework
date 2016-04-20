//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptRigidbody.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsScriptCollisionData.h"
#include "BsScriptSceneObject.h"
#include "BsScriptCollider.h"
#include "BsCollider.h"
#include "BsQuaternion.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptRigidbody::OnCollisionThunkDef ScriptRigidbody::onCollisionBeginThunk = nullptr;
	ScriptRigidbody::OnCollisionThunkDef ScriptRigidbody::onCollisionStayThunk = nullptr;
	ScriptRigidbody::OnCollisionThunkDef ScriptRigidbody::onCollisionEndThunk = nullptr;

	ScriptRigidbody::ScriptRigidbody(MonoObject* instance, const SPtr<Rigidbody>& body)
		:ScriptObject(instance), mRigidbody(body)
	{
		body->onCollisionBegin.connect(std::bind(&ScriptRigidbody::onCollisionBegin, instance, _1));
		body->onCollisionStay.connect(std::bind(&ScriptRigidbody::onCollisionStay, instance, _1));
		body->onCollisionEnd.connect(std::bind(&ScriptRigidbody::onCollisionEnd, instance, _1));
	}

	void ScriptRigidbody::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptRigidbody::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptRigidbody::internal_Destroy);

		metaData.scriptClass->addInternalCall("Internal_Move", &ScriptRigidbody::internal_Move);
		metaData.scriptClass->addInternalCall("Internal_Rotate", &ScriptRigidbody::internal_Rotate);

		metaData.scriptClass->addInternalCall("Internal_GetPosition", &ScriptRigidbody::internal_GetPosition);
		metaData.scriptClass->addInternalCall("Internal_GetRotation", &ScriptRigidbody::internal_GetRotation);
		metaData.scriptClass->addInternalCall("Internal_SetTransform", &ScriptRigidbody::internal_SetTransform);

		metaData.scriptClass->addInternalCall("Internal_SetMass", &ScriptRigidbody::internal_SetMass);
		metaData.scriptClass->addInternalCall("Internal_GetMass", &ScriptRigidbody::internal_GetMass);

		metaData.scriptClass->addInternalCall("Internal_SetIsKinematic", &ScriptRigidbody::internal_SetIsKinematic);
		metaData.scriptClass->addInternalCall("Internal_GetIsKinematic", &ScriptRigidbody::internal_GetIsKinematic);

		metaData.scriptClass->addInternalCall("Internal_IsSleeping", &ScriptRigidbody::internal_IsSleeping);
		metaData.scriptClass->addInternalCall("Internal_Sleep", &ScriptRigidbody::internal_Sleep);
		metaData.scriptClass->addInternalCall("Internal_WakeUp", &ScriptRigidbody::internal_WakeUp);

		metaData.scriptClass->addInternalCall("Internal_SetSleepThreshold", &ScriptRigidbody::internal_SetSleepThreshold);
		metaData.scriptClass->addInternalCall("Internal_GetSleepThreshold", &ScriptRigidbody::internal_GetSleepThreshold);

		metaData.scriptClass->addInternalCall("Internal_SetUseGravity", &ScriptRigidbody::internal_SetUseGravity);
		metaData.scriptClass->addInternalCall("Internal_GetUseGravity", &ScriptRigidbody::internal_GetUseGravity);

		metaData.scriptClass->addInternalCall("Internal_SetVelocity", &ScriptRigidbody::internal_SetVelocity);
		metaData.scriptClass->addInternalCall("Internal_GetVelocity", &ScriptRigidbody::internal_GetVelocity);

		metaData.scriptClass->addInternalCall("Internal_SetAngularVelocity", &ScriptRigidbody::internal_SetAngularVelocity);
		metaData.scriptClass->addInternalCall("Internal_GetAngularVelocity", &ScriptRigidbody::internal_GetAngularVelocity);

		metaData.scriptClass->addInternalCall("Internal_SetDrag", &ScriptRigidbody::internal_SetDrag);
		metaData.scriptClass->addInternalCall("Internal_GetDrag", &ScriptRigidbody::internal_GetDrag);

		metaData.scriptClass->addInternalCall("Internal_SetAngularDrag", &ScriptRigidbody::internal_SetAngularDrag);
		metaData.scriptClass->addInternalCall("Internal_GetAngularDrag", &ScriptRigidbody::internal_GetAngularDrag);

		metaData.scriptClass->addInternalCall("Internal_SetInertiaTensor", &ScriptRigidbody::internal_SetInertiaTensor);
		metaData.scriptClass->addInternalCall("Internal_GetInertiaTensor", &ScriptRigidbody::internal_GetInertiaTensor);

		metaData.scriptClass->addInternalCall("Internal_SetMaxAngularVelocity", &ScriptRigidbody::internal_SetMaxAngularVelocity);
		metaData.scriptClass->addInternalCall("Internal_GetMaxAngularVelocity", &ScriptRigidbody::internal_GetMaxAngularVelocity);

		metaData.scriptClass->addInternalCall("Internal_SetCenterOfMass", &ScriptRigidbody::internal_SetCenterOfMass);
		metaData.scriptClass->addInternalCall("Internal_GetCenterOfMassPosition", &ScriptRigidbody::internal_GetCenterOfMassPosition);
		metaData.scriptClass->addInternalCall("Internal_GetCenterOfMassRotation", &ScriptRigidbody::internal_GetCenterOfMassRotation);

		metaData.scriptClass->addInternalCall("Internal_SetPositionSolverCount", &ScriptRigidbody::internal_SetPositionSolverCount);
		metaData.scriptClass->addInternalCall("Internal_GetPositionSolverCount", &ScriptRigidbody::internal_GetPositionSolverCount);

		metaData.scriptClass->addInternalCall("Internal_SetVelocitySolverCount", &ScriptRigidbody::internal_SetVelocitySolverCount);
		metaData.scriptClass->addInternalCall("Internal_GetVelocitySolverCount", &ScriptRigidbody::internal_GetVelocitySolverCount);

		metaData.scriptClass->addInternalCall("Internal_SetFlags", &ScriptRigidbody::internal_SetFlags);
		metaData.scriptClass->addInternalCall("Internal_GetFlags", &ScriptRigidbody::internal_GetFlags);

		metaData.scriptClass->addInternalCall("Internal_AddForce", &ScriptRigidbody::internal_AddForce);
		metaData.scriptClass->addInternalCall("Internal_AddTorque", &ScriptRigidbody::internal_AddTorque);
		metaData.scriptClass->addInternalCall("Internal_AddForceAtPoint", &ScriptRigidbody::internal_AddForceAtPoint);

		metaData.scriptClass->addInternalCall("Internal_GetVelocityAtPoint", &ScriptRigidbody::internal_GetVelocityAtPoint);

		metaData.scriptClass->addInternalCall("Internal_AddCollider", &ScriptRigidbody::internal_AddCollider);
		metaData.scriptClass->addInternalCall("Internal_RemoveCollider", &ScriptRigidbody::internal_RemoveCollider);
		metaData.scriptClass->addInternalCall("Internal_RemoveColliders", &ScriptRigidbody::internal_RemoveColliders);

		metaData.scriptClass->addInternalCall("Internal_UpdateMassDistribution", &ScriptRigidbody::internal_UpdateMassDistribution);

		onCollisionBeginThunk = (OnCollisionThunkDef)metaData.scriptClass->getMethod("Internal_DoOnCollisionBegin", 1)->getThunk();
		onCollisionStayThunk = (OnCollisionThunkDef)metaData.scriptClass->getMethod("Internal_DoOnCollisionStay", 1)->getThunk();
		onCollisionEndThunk = (OnCollisionThunkDef)metaData.scriptClass->getMethod("Internal_DoOnCollisionEnd", 1)->getThunk();
	}

	void ScriptRigidbody::onCollisionBegin(MonoObject* instance, const CollisionData& collisionData)
	{
		MonoObject* managedCollisionData = ScriptCollisionDataHelper::box(ScriptCollisionDataHelper::create(collisionData));
		MonoUtil::invokeThunk(onCollisionBeginThunk, instance, managedCollisionData);
	}

	void ScriptRigidbody::onCollisionStay(MonoObject* instance, const CollisionData& collisionData)
	{
		MonoObject* managedCollisionData = ScriptCollisionDataHelper::box(ScriptCollisionDataHelper::create(collisionData));
		MonoUtil::invokeThunk(onCollisionStayThunk, instance, managedCollisionData);
	}

	void ScriptRigidbody::onCollisionEnd(MonoObject* instance, const CollisionData& collisionData)
	{
		MonoObject* managedCollisionData = ScriptCollisionDataHelper::box(ScriptCollisionDataHelper::create(collisionData));
		MonoUtil::invokeThunk(onCollisionEndThunk, instance, managedCollisionData);
	}

	void ScriptRigidbody::internal_CreateInstance(MonoObject* instance, ScriptSceneObject* linkedSO)
	{
		HSceneObject so;
		if (linkedSO != nullptr)
			so = linkedSO->getNativeSceneObject();

		SPtr<Rigidbody> rigidbody = Rigidbody::create(so);
		rigidbody->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptRigidbody>()) ScriptRigidbody(instance, rigidbody);
	}

	void ScriptRigidbody::internal_Destroy(ScriptRigidbody* thisPtr)
	{
		thisPtr->mRigidbody = nullptr;
	}

	void ScriptRigidbody::internal_Move(ScriptRigidbody* thisPtr, Vector3* position)
	{
		thisPtr->mRigidbody->move(*position);
	}

	void ScriptRigidbody::internal_Rotate(ScriptRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->mRigidbody->rotate(*rotation);
	}

	void ScriptRigidbody::internal_GetPosition(ScriptRigidbody* thisPtr, Vector3* position)
	{
		*position = thisPtr->mRigidbody->getPosition();
	}

	void ScriptRigidbody::internal_GetRotation(ScriptRigidbody* thisPtr, Quaternion* rotation)
	{
		*rotation = thisPtr->mRigidbody->getRotation();
	}

	void ScriptRigidbody::internal_SetTransform(ScriptRigidbody* thisPtr, Vector3* pos, Quaternion* rot)
	{
		thisPtr->mRigidbody->setTransform(*pos, *rot);
	}

	void ScriptRigidbody::internal_SetMass(ScriptRigidbody* thisPtr, float mass)
	{
		thisPtr->mRigidbody->setMass(mass);
	}

	float ScriptRigidbody::internal_GetMass(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getMass();
	}

	void ScriptRigidbody::internal_SetIsKinematic(ScriptRigidbody* thisPtr, bool kinematic)
	{
		thisPtr->mRigidbody->setIsKinematic(kinematic);
	}

	bool ScriptRigidbody::internal_GetIsKinematic(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getIsKinematic();
	}

	bool ScriptRigidbody::internal_IsSleeping(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->isSleeping();
	}

	void ScriptRigidbody::internal_Sleep(ScriptRigidbody* thisPtr)
	{
		thisPtr->mRigidbody->sleep();
	}

	void ScriptRigidbody::internal_WakeUp(ScriptRigidbody* thisPtr)
	{
		thisPtr->mRigidbody->wakeUp();
	}

	void ScriptRigidbody::internal_SetSleepThreshold(ScriptRigidbody* thisPtr, float threshold)
	{
		thisPtr->mRigidbody->setSleepThreshold(threshold);
	}

	float ScriptRigidbody::internal_GetSleepThreshold(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getSleepThreshold();
	}

	void ScriptRigidbody::internal_SetUseGravity(ScriptRigidbody* thisPtr, bool gravity)
	{
		thisPtr->mRigidbody->setUseGravity(gravity);
	}

	bool ScriptRigidbody::internal_GetUseGravity(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getUseGravity();
	}

	void ScriptRigidbody::internal_SetVelocity(ScriptRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->mRigidbody->setVelocity(*velocity);
	}

	void ScriptRigidbody::internal_GetVelocity(ScriptRigidbody* thisPtr, Vector3* velocity)
	{
		*velocity = thisPtr->mRigidbody->getVelocity();
	}

	void ScriptRigidbody::internal_SetAngularVelocity(ScriptRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->mRigidbody->setAngularVelocity(*velocity);
	}

	void ScriptRigidbody::internal_GetAngularVelocity(ScriptRigidbody* thisPtr, Vector3* velocity)
	{
		*velocity = thisPtr->mRigidbody->getAngularVelocity();
	}

	void ScriptRigidbody::internal_SetDrag(ScriptRigidbody* thisPtr, float drag)
	{
		thisPtr->mRigidbody->setDrag(drag);
	}

	float ScriptRigidbody::internal_GetDrag(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getDrag();
	}

	void ScriptRigidbody::internal_SetAngularDrag(ScriptRigidbody* thisPtr, float drag)
	{
		thisPtr->mRigidbody->setAngularDrag(drag);
	}

	float ScriptRigidbody::internal_GetAngularDrag(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getAngularDrag();
	}

	void ScriptRigidbody::internal_SetInertiaTensor(ScriptRigidbody* thisPtr, Vector3* tensor)
	{
		thisPtr->mRigidbody->setInertiaTensor(*tensor);
	}

	void ScriptRigidbody::internal_GetInertiaTensor(ScriptRigidbody* thisPtr, Vector3* tensor)
	{
		*tensor = thisPtr->mRigidbody->getInertiaTensor();
	}

	void ScriptRigidbody::internal_SetMaxAngularVelocity(ScriptRigidbody* thisPtr, float maxVelocity)
	{
		thisPtr->mRigidbody->setMaxAngularVelocity(maxVelocity);
	}

	float ScriptRigidbody::internal_GetMaxAngularVelocity(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getMaxAngularVelocity();
	}

	void ScriptRigidbody::internal_SetCenterOfMass(ScriptRigidbody* thisPtr, Vector3* position, Quaternion* rotation)
	{
		thisPtr->mRigidbody->setCenterOfMass(*position, *rotation);
	}

	void ScriptRigidbody::internal_GetCenterOfMassPosition(ScriptRigidbody* thisPtr, Vector3* position)
	{
		*position = thisPtr->mRigidbody->getCenterOfMassPosition();
	}

	void ScriptRigidbody::internal_GetCenterOfMassRotation(ScriptRigidbody* thisPtr, Quaternion* rotation)
	{
		*rotation = thisPtr->mRigidbody->getCenterOfMassRotation();
	}

	void ScriptRigidbody::internal_SetPositionSolverCount(ScriptRigidbody* thisPtr, UINT32 count)
	{
		thisPtr->mRigidbody->setPositionSolverCount(count);
	}

	UINT32 ScriptRigidbody::internal_GetPositionSolverCount(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getPositionSolverCount();
	}

	void ScriptRigidbody::internal_SetVelocitySolverCount(ScriptRigidbody* thisPtr, UINT32 count)
	{
		thisPtr->mRigidbody->setVelocitySolverCount(count);
	}

	UINT32 ScriptRigidbody::internal_GetVelocitySolverCount(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getVelocitySolverCount();
	}

	void ScriptRigidbody::internal_SetFlags(ScriptRigidbody* thisPtr, Rigidbody::Flag flags)
	{
		thisPtr->mRigidbody->setFlags(flags);
	}

	Rigidbody::Flag ScriptRigidbody::internal_GetFlags(ScriptRigidbody* thisPtr)
	{
		return thisPtr->mRigidbody->getFlags();
	}

	void ScriptRigidbody::internal_AddForce(ScriptRigidbody* thisPtr, Vector3* force, ForceMode mode)
	{
		thisPtr->mRigidbody->addForce(*force, mode);
	}

	void ScriptRigidbody::internal_AddTorque(ScriptRigidbody* thisPtr, Vector3* torque, ForceMode mode)
	{
		thisPtr->mRigidbody->addTorque(*torque, mode);
	}

	void ScriptRigidbody::internal_AddForceAtPoint(ScriptRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode)
	{
		thisPtr->mRigidbody->addForceAtPoint(*force, *position, mode);
	}

	void ScriptRigidbody::internal_GetVelocityAtPoint(ScriptRigidbody* thisPtr, Vector3* point, Vector3* velocity)
	{
		*velocity = thisPtr->mRigidbody->getVelocityAtPoint(*point);
	}

	void ScriptRigidbody::internal_AddCollider(ScriptRigidbody* thisPtr, ScriptColliderBase* collider)
	{
		if (collider == nullptr)
			return;

		thisPtr->mRigidbody->addCollider(collider->getCollider()->_getInternal());
	}

	void ScriptRigidbody::internal_RemoveCollider(ScriptRigidbody* thisPtr, ScriptColliderBase* collider)
	{
		if (collider == nullptr)
			return;

		thisPtr->mRigidbody->removeCollider(collider->getCollider()->_getInternal());
	}

	void ScriptRigidbody::internal_RemoveColliders(ScriptRigidbody* thisPtr)
	{
		thisPtr->mRigidbody->removeColliders();
	}

	void ScriptRigidbody::internal_UpdateMassDistribution(ScriptRigidbody* thisPtr)
	{
		thisPtr->mRigidbody->updateMassDistribution();
	}
}