//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCRigidbody.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCRigidbody.h"
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
		value->onCollisionBegin.Connect(std::bind(&ScriptCRigidbody::onCollisionBegin, this, std::placeholders::_1));
		value->onCollisionStay.Connect(std::bind(&ScriptCRigidbody::onCollisionStay, this, std::placeholders::_1));
		value->onCollisionEnd.Connect(std::bind(&ScriptCRigidbody::onCollisionEnd, this, std::placeholders::_1));
	}

	void ScriptCRigidbody::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_move", (void*)&ScriptCRigidbody::Internal_move);
		metaData.scriptClass->AddInternalCall("Internal_rotate", (void*)&ScriptCRigidbody::Internal_rotate);
		metaData.scriptClass->AddInternalCall("Internal_setMass", (void*)&ScriptCRigidbody::Internal_setMass);
		metaData.scriptClass->AddInternalCall("Internal_getMass", (void*)&ScriptCRigidbody::Internal_getMass);
		metaData.scriptClass->AddInternalCall("Internal_setIsKinematic", (void*)&ScriptCRigidbody::Internal_setIsKinematic);
		metaData.scriptClass->AddInternalCall("Internal_getIsKinematic", (void*)&ScriptCRigidbody::Internal_getIsKinematic);
		metaData.scriptClass->AddInternalCall("Internal_isSleeping", (void*)&ScriptCRigidbody::Internal_isSleeping);
		metaData.scriptClass->AddInternalCall("Internal_sleep", (void*)&ScriptCRigidbody::Internal_sleep);
		metaData.scriptClass->AddInternalCall("Internal_wakeUp", (void*)&ScriptCRigidbody::Internal_wakeUp);
		metaData.scriptClass->AddInternalCall("Internal_setSleepThreshold", (void*)&ScriptCRigidbody::Internal_setSleepThreshold);
		metaData.scriptClass->AddInternalCall("Internal_getSleepThreshold", (void*)&ScriptCRigidbody::Internal_getSleepThreshold);
		metaData.scriptClass->AddInternalCall("Internal_setUseGravity", (void*)&ScriptCRigidbody::Internal_setUseGravity);
		metaData.scriptClass->AddInternalCall("Internal_getUseGravity", (void*)&ScriptCRigidbody::Internal_getUseGravity);
		metaData.scriptClass->AddInternalCall("Internal_setVelocity", (void*)&ScriptCRigidbody::Internal_setVelocity);
		metaData.scriptClass->AddInternalCall("Internal_getVelocity", (void*)&ScriptCRigidbody::Internal_getVelocity);
		metaData.scriptClass->AddInternalCall("Internal_setAngularVelocity", (void*)&ScriptCRigidbody::Internal_setAngularVelocity);
		metaData.scriptClass->AddInternalCall("Internal_getAngularVelocity", (void*)&ScriptCRigidbody::Internal_getAngularVelocity);
		metaData.scriptClass->AddInternalCall("Internal_setDrag", (void*)&ScriptCRigidbody::Internal_setDrag);
		metaData.scriptClass->AddInternalCall("Internal_getDrag", (void*)&ScriptCRigidbody::Internal_getDrag);
		metaData.scriptClass->AddInternalCall("Internal_setAngularDrag", (void*)&ScriptCRigidbody::Internal_setAngularDrag);
		metaData.scriptClass->AddInternalCall("Internal_getAngularDrag", (void*)&ScriptCRigidbody::Internal_getAngularDrag);
		metaData.scriptClass->AddInternalCall("Internal_setInertiaTensor", (void*)&ScriptCRigidbody::Internal_setInertiaTensor);
		metaData.scriptClass->AddInternalCall("Internal_getInertiaTensor", (void*)&ScriptCRigidbody::Internal_getInertiaTensor);
		metaData.scriptClass->AddInternalCall("Internal_setMaxAngularVelocity", (void*)&ScriptCRigidbody::Internal_setMaxAngularVelocity);
		metaData.scriptClass->AddInternalCall("Internal_getMaxAngularVelocity", (void*)&ScriptCRigidbody::Internal_getMaxAngularVelocity);
		metaData.scriptClass->AddInternalCall("Internal_setCenterOfMassPosition", (void*)&ScriptCRigidbody::Internal_setCenterOfMassPosition);
		metaData.scriptClass->AddInternalCall("Internal_getCenterOfMassPosition", (void*)&ScriptCRigidbody::Internal_getCenterOfMassPosition);
		metaData.scriptClass->AddInternalCall("Internal_setCenterOfMassRotation", (void*)&ScriptCRigidbody::Internal_setCenterOfMassRotation);
		metaData.scriptClass->AddInternalCall("Internal_getCenterOfMassRotation", (void*)&ScriptCRigidbody::Internal_getCenterOfMassRotation);
		metaData.scriptClass->AddInternalCall("Internal_setPositionSolverCount", (void*)&ScriptCRigidbody::Internal_setPositionSolverCount);
		metaData.scriptClass->AddInternalCall("Internal_getPositionSolverCount", (void*)&ScriptCRigidbody::Internal_getPositionSolverCount);
		metaData.scriptClass->AddInternalCall("Internal_setVelocitySolverCount", (void*)&ScriptCRigidbody::Internal_setVelocitySolverCount);
		metaData.scriptClass->AddInternalCall("Internal_getVelocitySolverCount", (void*)&ScriptCRigidbody::Internal_getVelocitySolverCount);
		metaData.scriptClass->AddInternalCall("Internal_setCollisionReportMode", (void*)&ScriptCRigidbody::Internal_setCollisionReportMode);
		metaData.scriptClass->AddInternalCall("Internal_getCollisionReportMode", (void*)&ScriptCRigidbody::Internal_getCollisionReportMode);
		metaData.scriptClass->AddInternalCall("Internal_setFlags", (void*)&ScriptCRigidbody::Internal_setFlags);
		metaData.scriptClass->AddInternalCall("Internal_getFlags", (void*)&ScriptCRigidbody::Internal_getFlags);
		metaData.scriptClass->AddInternalCall("Internal_addForce", (void*)&ScriptCRigidbody::Internal_addForce);
		metaData.scriptClass->AddInternalCall("Internal_addTorque", (void*)&ScriptCRigidbody::Internal_addTorque);
		metaData.scriptClass->AddInternalCall("Internal_addForceAtPoint", (void*)&ScriptCRigidbody::Internal_addForceAtPoint);
		metaData.scriptClass->AddInternalCall("Internal_getVelocityAtPoint", (void*)&ScriptCRigidbody::Internal_getVelocityAtPoint);

		onCollisionBeginThunk = (onCollisionBeginThunkDef)metaData.scriptClass->GetMethodExact("Internal_onCollisionBegin", "CollisionData&")->getThunk();
		onCollisionStayThunk = (onCollisionStayThunkDef)metaData.scriptClass->GetMethodExact("Internal_onCollisionStay", "CollisionData&")->getThunk();
		onCollisionEndThunk = (onCollisionEndThunkDef)metaData.scriptClass->GetMethodExact("Internal_onCollisionEnd", "CollisionData&")->getThunk();
	}

	void ScriptCRigidbody::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionBeginThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCRigidbody::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionStayThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCRigidbody::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::toInterop(p0);
		tmpp0 = ScriptCollisionData::box(interopp0);
		MonoUtil::invokeThunk(onCollisionEndThunk, getManagedInstance(), tmpp0);
	}
	void ScriptCRigidbody::Internal_move(ScriptCRigidbody* thisPtr, Vector3* position)
	{
		thisPtr->GetHandle()->move(*position);
	}

	void ScriptCRigidbody::Internal_rotate(ScriptCRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->GetHandle()->rotate(*rotation);
	}

	void ScriptCRigidbody::Internal_setMass(ScriptCRigidbody* thisPtr, float mass)
	{
		thisPtr->GetHandle()->setMass(mass);
	}

	float ScriptCRigidbody::Internal_getMass(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setIsKinematic(ScriptCRigidbody* thisPtr, bool kinematic)
	{
		thisPtr->GetHandle()->setIsKinematic(kinematic);
	}

	bool ScriptCRigidbody::Internal_getIsKinematic(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->getIsKinematic();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCRigidbody::Internal_isSleeping(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->isSleeping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_sleep(ScriptCRigidbody* thisPtr)
	{
		thisPtr->GetHandle()->sleep();
	}

	void ScriptCRigidbody::Internal_wakeUp(ScriptCRigidbody* thisPtr)
	{
		thisPtr->GetHandle()->wakeUp();
	}

	void ScriptCRigidbody::Internal_setSleepThreshold(ScriptCRigidbody* thisPtr, float threshold)
	{
		thisPtr->GetHandle()->setSleepThreshold(threshold);
	}

	float ScriptCRigidbody::Internal_getSleepThreshold(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getSleepThreshold();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setUseGravity(ScriptCRigidbody* thisPtr, bool gravity)
	{
		thisPtr->GetHandle()->setUseGravity(gravity);
	}

	bool ScriptCRigidbody::Internal_getUseGravity(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->getUseGravity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->GetHandle()->setVelocity(*velocity);
	}

	void ScriptCRigidbody::Internal_getVelocity(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getVelocity();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->GetHandle()->setAngularVelocity(*velocity);
	}

	void ScriptCRigidbody::Internal_getAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setDrag(ScriptCRigidbody* thisPtr, float drag)
	{
		thisPtr->GetHandle()->setDrag(drag);
	}

	float ScriptCRigidbody::Internal_getDrag(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setAngularDrag(ScriptCRigidbody* thisPtr, float drag)
	{
		thisPtr->GetHandle()->setAngularDrag(drag);
	}

	float ScriptCRigidbody::Internal_getAngularDrag(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getAngularDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* tensor)
	{
		thisPtr->GetHandle()->setInertiaTensor(*tensor);
	}

	void ScriptCRigidbody::Internal_getInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getInertiaTensor();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setMaxAngularVelocity(ScriptCRigidbody* thisPtr, float maxVelocity)
	{
		thisPtr->GetHandle()->setMaxAngularVelocity(maxVelocity);
	}

	float ScriptCRigidbody::Internal_getMaxAngularVelocity(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getMaxAngularVelocity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* position)
	{
		thisPtr->GetHandle()->setCenterOfMassPosition(*position);
	}

	void ScriptCRigidbody::Internal_getCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getCenterOfMassPosition();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->GetHandle()->setCenterOfMassRotation(*rotation);
	}

	void ScriptCRigidbody::Internal_getCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetHandle()->getCenterOfMassRotation();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::Internal_setPositionSolverCount(ScriptCRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->setPositionSolverCount(count);
	}

	uint32_t ScriptCRigidbody::Internal_getPositionSolverCount(ScriptCRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getPositionSolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setVelocitySolverCount(ScriptCRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->setVelocitySolverCount(count);
	}

	uint32_t ScriptCRigidbody::Internal_getVelocitySolverCount(ScriptCRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getVelocitySolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setCollisionReportMode(ScriptCRigidbody* thisPtr, CollisionReportMode mode)
	{
		thisPtr->GetHandle()->setCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCRigidbody::Internal_getCollisionReportMode(ScriptCRigidbody* thisPtr)
	{
		CollisionReportMode tmp__output;
		tmp__output = thisPtr->GetHandle()->getCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_setFlags(ScriptCRigidbody* thisPtr, RigidbodyFlag flags)
	{
		thisPtr->GetHandle()->setFlags(flags);
	}

	RigidbodyFlag ScriptCRigidbody::Internal_getFlags(ScriptCRigidbody* thisPtr)
	{
		RigidbodyFlag tmp__output;
		tmp__output = thisPtr->GetHandle()->getFlags();

		RigidbodyFlag __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::Internal_addForce(ScriptCRigidbody* thisPtr, Vector3* force, ForceMode mode)
	{
		thisPtr->GetHandle()->addForce(*force, mode);
	}

	void ScriptCRigidbody::Internal_addTorque(ScriptCRigidbody* thisPtr, Vector3* torque, ForceMode mode)
	{
		thisPtr->GetHandle()->addTorque(*torque, mode);
	}

	void ScriptCRigidbody::Internal_addForceAtPoint(ScriptCRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode)
	{
		thisPtr->GetHandle()->addForceAtPoint(*force, *position, mode);
	}

	void ScriptCRigidbody::Internal_getVelocityAtPoint(ScriptCRigidbody* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getVelocityAtPoint(*point);

		*__output = tmp__output;
	}
}
