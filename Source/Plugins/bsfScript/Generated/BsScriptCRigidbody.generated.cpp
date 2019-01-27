//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCRigidbody.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCRigidbody.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"
#include "BsScriptCollisionData.generated.h"

namespace bs
{
	ScriptCRigidbody::onCollisionBeginThunkDef ScriptCRigidbody::onCollisionBeginThunk; 
	ScriptCRigidbody::onCollisionStayThunkDef ScriptCRigidbody::onCollisionStayThunk; 
	ScriptCRigidbody::onCollisionEndThunkDef ScriptCRigidbody::onCollisionEndThunk; 

	ScriptCRigidbody::ScriptCRigidbody(MonoObject* managedInstance, const GameObjectHandle<CRigidbody>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->onCollisionBegin.connect(std::bind(&ScriptCRigidbody::onCollisionBegin, this, std::placeholders::_1));
		value->onCollisionStay.connect(std::bind(&ScriptCRigidbody::onCollisionStay, this, std::placeholders::_1));
		value->onCollisionEnd.connect(std::bind(&ScriptCRigidbody::onCollisionEnd, this, std::placeholders::_1));
	}

	void ScriptCRigidbody::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_move", (void*)&ScriptCRigidbody::Internal_move);
		metaData.scriptClass->addInternalCall("Internal_rotate", (void*)&ScriptCRigidbody::Internal_rotate);
		metaData.scriptClass->addInternalCall("Internal_setMass", (void*)&ScriptCRigidbody::Internal_setMass);
		metaData.scriptClass->addInternalCall("Internal_getMass", (void*)&ScriptCRigidbody::Internal_getMass);
		metaData.scriptClass->addInternalCall("Internal_setIsKinematic", (void*)&ScriptCRigidbody::Internal_setIsKinematic);
		metaData.scriptClass->addInternalCall("Internal_getIsKinematic", (void*)&ScriptCRigidbody::Internal_getIsKinematic);
		metaData.scriptClass->addInternalCall("Internal_isSleeping", (void*)&ScriptCRigidbody::Internal_isSleeping);
		metaData.scriptClass->addInternalCall("Internal_sleep", (void*)&ScriptCRigidbody::Internal_sleep);
		metaData.scriptClass->addInternalCall("Internal_wakeUp", (void*)&ScriptCRigidbody::Internal_wakeUp);
		metaData.scriptClass->addInternalCall("Internal_setSleepThreshold", (void*)&ScriptCRigidbody::Internal_setSleepThreshold);
		metaData.scriptClass->addInternalCall("Internal_getSleepThreshold", (void*)&ScriptCRigidbody::Internal_getSleepThreshold);
		metaData.scriptClass->addInternalCall("Internal_setUseGravity", (void*)&ScriptCRigidbody::Internal_setUseGravity);
		metaData.scriptClass->addInternalCall("Internal_getUseGravity", (void*)&ScriptCRigidbody::Internal_getUseGravity);
		metaData.scriptClass->addInternalCall("Internal_setVelocity", (void*)&ScriptCRigidbody::Internal_setVelocity);
		metaData.scriptClass->addInternalCall("Internal_getVelocity", (void*)&ScriptCRigidbody::Internal_getVelocity);
		metaData.scriptClass->addInternalCall("Internal_setAngularVelocity", (void*)&ScriptCRigidbody::Internal_setAngularVelocity);
		metaData.scriptClass->addInternalCall("Internal_getAngularVelocity", (void*)&ScriptCRigidbody::Internal_getAngularVelocity);
		metaData.scriptClass->addInternalCall("Internal_setDrag", (void*)&ScriptCRigidbody::Internal_setDrag);
		metaData.scriptClass->addInternalCall("Internal_getDrag", (void*)&ScriptCRigidbody::Internal_getDrag);
		metaData.scriptClass->addInternalCall("Internal_setAngularDrag", (void*)&ScriptCRigidbody::Internal_setAngularDrag);
		metaData.scriptClass->addInternalCall("Internal_getAngularDrag", (void*)&ScriptCRigidbody::Internal_getAngularDrag);
		metaData.scriptClass->addInternalCall("Internal_setInertiaTensor", (void*)&ScriptCRigidbody::Internal_setInertiaTensor);
		metaData.scriptClass->addInternalCall("Internal_getInertiaTensor", (void*)&ScriptCRigidbody::Internal_getInertiaTensor);
		metaData.scriptClass->addInternalCall("Internal_setMaxAngularVelocity", (void*)&ScriptCRigidbody::Internal_setMaxAngularVelocity);
		metaData.scriptClass->addInternalCall("Internal_getMaxAngularVelocity", (void*)&ScriptCRigidbody::Internal_getMaxAngularVelocity);
		metaData.scriptClass->addInternalCall("Internal_setCenterOfMassPosition", (void*)&ScriptCRigidbody::Internal_setCenterOfMassPosition);
		metaData.scriptClass->addInternalCall("Internal_getCenterOfMassPosition", (void*)&ScriptCRigidbody::Internal_getCenterOfMassPosition);
		metaData.scriptClass->addInternalCall("Internal_setCenterOfMassRotation", (void*)&ScriptCRigidbody::Internal_setCenterOfMassRotation);
		metaData.scriptClass->addInternalCall("Internal_getCenterOfMassRotation", (void*)&ScriptCRigidbody::Internal_getCenterOfMassRotation);
		metaData.scriptClass->addInternalCall("Internal_setPositionSolverCount", (void*)&ScriptCRigidbody::Internal_setPositionSolverCount);
		metaData.scriptClass->addInternalCall("Internal_getPositionSolverCount", (void*)&ScriptCRigidbody::Internal_getPositionSolverCount);
		metaData.scriptClass->addInternalCall("Internal_setVelocitySolverCount", (void*)&ScriptCRigidbody::Internal_setVelocitySolverCount);
		metaData.scriptClass->addInternalCall("Internal_getVelocitySolverCount", (void*)&ScriptCRigidbody::Internal_getVelocitySolverCount);
		metaData.scriptClass->addInternalCall("Internal_setCollisionReportMode", (void*)&ScriptCRigidbody::Internal_setCollisionReportMode);
		metaData.scriptClass->addInternalCall("Internal_getCollisionReportMode", (void*)&ScriptCRigidbody::Internal_getCollisionReportMode);
		metaData.scriptClass->addInternalCall("Internal_setFlags", (void*)&ScriptCRigidbody::Internal_setFlags);
		metaData.scriptClass->addInternalCall("Internal_getFlags", (void*)&ScriptCRigidbody::Internal_getFlags);
		metaData.scriptClass->addInternalCall("Internal_addForce", (void*)&ScriptCRigidbody::Internal_addForce);
		metaData.scriptClass->addInternalCall("Internal_addTorque", (void*)&ScriptCRigidbody::Internal_addTorque);
		metaData.scriptClass->addInternalCall("Internal_addForceAtPoint", (void*)&ScriptCRigidbody::Internal_addForceAtPoint);
		metaData.scriptClass->addInternalCall("Internal_getVelocityAtPoint", (void*)&ScriptCRigidbody::Internal_getVelocityAtPoint);

		onCollisionBeginThunk = (onCollisionBeginThunkDef)metaData.scriptClass->getMethodExact("Internal_onCollisionBegin", "CollisionData&")->getThunk();
		onCollisionStayThunk = (onCollisionStayThunkDef)metaData.scriptClass->getMethodExact("Internal_onCollisionStay", "CollisionData&")->getThunk();
		onCollisionEndThunk = (onCollisionEndThunkDef)metaData.scriptClass->getMethodExact("Internal_onCollisionEnd", "CollisionData&")->getThunk();
	}

	void ScriptCRigidbody::onCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionBeginThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCRigidbody::onCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionStayThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCRigidbody::onCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionEndThunk, getManagedInstance(), tmpp0);
	}
	void ScriptCRigidbody::Internal_move(ScriptCRigidbody* thisPtr, Vector3* position)
	{
		thisPtr->getHandle()->move(*position);
	}

	void ScriptCRigidbody::Internal_rotate(ScriptCRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->getHandle()->rotate(*rotation);
	}

	void ScriptCRigidbody::Internal_setMass(ScriptCRigidbody* thisPtr, float mass)
	{
		thisPtr->getHandle()->setMass(mass);
	}

	float ScriptCRigidbody::Internal_getMass(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setIsKinematic(ScriptCRigidbody* thisPtr, bool kinematic)
	{
		thisPtr->getHandle()->setIsKinematic(kinematic);
	}

	bool ScriptCRigidbody::Internal_getIsKinematic(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getIsKinematic();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCRigidbody::Internal_isSleeping(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->isSleeping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_sleep(ScriptCRigidbody* thisPtr)
	{
		thisPtr->getHandle()->sleep();
	}

	void ScriptCRigidbody::Internal_wakeUp(ScriptCRigidbody* thisPtr)
	{
		thisPtr->getHandle()->wakeUp();
	}

	void ScriptCRigidbody::Internal_setSleepThreshold(ScriptCRigidbody* thisPtr, float threshold)
	{
		thisPtr->getHandle()->setSleepThreshold(threshold);
	}

	float ScriptCRigidbody::Internal_getSleepThreshold(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getSleepThreshold();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setUseGravity(ScriptCRigidbody* thisPtr, bool gravity)
	{
		thisPtr->getHandle()->setUseGravity(gravity);
	}

	bool ScriptCRigidbody::Internal_getUseGravity(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getUseGravity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->getHandle()->setVelocity(*velocity);
	}

	void ScriptCRigidbody::Internal_getVelocity(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getVelocity();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->getHandle()->setAngularVelocity(*velocity);
	}

	void ScriptCRigidbody::Internal_getAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setDrag(ScriptCRigidbody* thisPtr, float drag)
	{
		thisPtr->getHandle()->setDrag(drag);
	}

	float ScriptCRigidbody::Internal_getDrag(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setAngularDrag(ScriptCRigidbody* thisPtr, float drag)
	{
		thisPtr->getHandle()->setAngularDrag(drag);
	}

	float ScriptCRigidbody::Internal_getAngularDrag(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getAngularDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* tensor)
	{
		thisPtr->getHandle()->setInertiaTensor(*tensor);
	}

	void ScriptCRigidbody::Internal_getInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getInertiaTensor();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setMaxAngularVelocity(ScriptCRigidbody* thisPtr, float maxVelocity)
	{
		thisPtr->getHandle()->setMaxAngularVelocity(maxVelocity);
	}

	float ScriptCRigidbody::Internal_getMaxAngularVelocity(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getMaxAngularVelocity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* position)
	{
		thisPtr->getHandle()->setCenterOfMassPosition(*position);
	}

	void ScriptCRigidbody::Internal_getCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getCenterOfMassPosition();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->getHandle()->setCenterOfMassRotation(*rotation);
	}

	void ScriptCRigidbody::Internal_getCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->getHandle()->getCenterOfMassRotation();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setPositionSolverCount(ScriptCRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->getHandle()->setPositionSolverCount(count);
	}

	uint32_t ScriptCRigidbody::Internal_getPositionSolverCount(ScriptCRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getPositionSolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setVelocitySolverCount(ScriptCRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->getHandle()->setVelocitySolverCount(count);
	}

	uint32_t ScriptCRigidbody::Internal_getVelocitySolverCount(ScriptCRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getVelocitySolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setCollisionReportMode(ScriptCRigidbody* thisPtr, CollisionReportMode mode)
	{
		thisPtr->getHandle()->setCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCRigidbody::Internal_getCollisionReportMode(ScriptCRigidbody* thisPtr)
	{
		CollisionReportMode tmp__output;
		tmp__output = thisPtr->getHandle()->getCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setFlags(ScriptCRigidbody* thisPtr, RigidbodyFlag flags)
	{
		thisPtr->getHandle()->setFlags(flags);
	}

	RigidbodyFlag ScriptCRigidbody::Internal_getFlags(ScriptCRigidbody* thisPtr)
	{
		RigidbodyFlag tmp__output;
		tmp__output = thisPtr->getHandle()->getFlags();

		RigidbodyFlag __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_addForce(ScriptCRigidbody* thisPtr, Vector3* force, ForceMode mode)
	{
		thisPtr->getHandle()->addForce(*force, mode);
	}

	void ScriptCRigidbody::Internal_addTorque(ScriptCRigidbody* thisPtr, Vector3* torque, ForceMode mode)
	{
		thisPtr->getHandle()->addTorque(*torque, mode);
	}

	void ScriptCRigidbody::Internal_addForceAtPoint(ScriptCRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode)
	{
		thisPtr->getHandle()->addForceAtPoint(*force, *position, mode);
	}

	void ScriptCRigidbody::Internal_getVelocityAtPoint(ScriptCRigidbody* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getVelocityAtPoint(*point);

		*__output = tmp__output;
	}
}
