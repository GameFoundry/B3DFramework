//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptJoint.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsScriptRigidbody.h"

namespace bs
{
	ScriptJoint::OnJointBreakThunkDef ScriptJoint::onJointBreakThunk = nullptr;

	ScriptJointBase::ScriptJointBase(MonoObject* instance)
		:ScriptObjectBase(instance)
	{ }

	void ScriptJointBase::initialize(const SPtr<Joint>& joint)
	{
		mJoint = joint;

		MonoObject* instance = getManagedInstance();
		joint->onJointBreak.connect(std::bind(&ScriptJoint::onJointBreak, instance));
	}

	void ScriptJointBase::destroyJoint()
	{
		mJoint = nullptr;
	}

	ScriptJoint::ScriptJoint(MonoObject* instance)
		:TScriptJoint(instance, nullptr)
	{ }

	void ScriptJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptJoint::internal_Destroy);
		metaData.scriptClass->addInternalCall("Internal_SetBody", &ScriptJoint::internal_SetBody);
		metaData.scriptClass->addInternalCall("Internal_SetPosition", &ScriptJoint::internal_SetPosition);
		metaData.scriptClass->addInternalCall("Internal_SetRotation", &ScriptJoint::internal_SetRotation);
		metaData.scriptClass->addInternalCall("Internal_SetBreakForce", &ScriptJoint::internal_SetBreakForce);
		metaData.scriptClass->addInternalCall("Internal_SetBreakTorque", &ScriptJoint::internal_SetBreakTorque);
		metaData.scriptClass->addInternalCall("Internal_SetEnableCollision", &ScriptJoint::internal_SetEnableCollision);

		onJointBreakThunk = (OnJointBreakThunkDef)metaData.scriptClass->getMethod("Internal_DoOnJointBreak")->getThunk();
	}

	void ScriptJoint::onJointBreak(MonoObject* instance)
	{
		MonoUtil::invokeThunk(onJointBreakThunk, instance);
	}

	void ScriptJoint::internal_Destroy(ScriptJointBase* thisPtr)
	{
		thisPtr->destroyJoint();
	}

	void ScriptJoint::internal_SetBody(ScriptJointBase* thisPtr, JointBody body, ScriptRigidbody* value)
	{
		Rigidbody* rigidbody = nullptr;
		if (value != nullptr)
			rigidbody = value->getRigidbody();

		thisPtr->mJoint->setBody(body, rigidbody);
	}

	void ScriptJoint::internal_SetPosition(ScriptJointBase* thisPtr, JointBody body, Vector3* position)
	{
		Quaternion rotation = thisPtr->mJoint->getRotation(body);
		thisPtr->mJoint->setTransform(body, *position, rotation);
	}

	void ScriptJoint::internal_SetRotation(ScriptJointBase* thisPtr, JointBody body, Quaternion* rotation)
	{
		Vector3 position = thisPtr->mJoint->getPosition(body);
		thisPtr->mJoint->setTransform(body, position, *rotation);
	}

	void ScriptJoint::internal_SetBreakForce(ScriptJointBase* thisPtr, float force)
	{
		thisPtr->mJoint->setBreakForce(force);
	}

	void ScriptJoint::internal_SetBreakTorque(ScriptJointBase* thisPtr, float torque)
	{
		thisPtr->mJoint->setBreakTorque(torque);
	}

	void ScriptJoint::internal_SetEnableCollision(ScriptJointBase* thisPtr, bool value)
	{
		thisPtr->mJoint->setEnableCollision(value);
	}

	void ScriptCommonJointData::toDesc(JOINT_DESC& desc) const
	{
		ScriptArray bodiesArray(bodies);
		ScriptRigidbody* rigidbodyA = bodiesArray.get<ScriptRigidbody*>(0);
		if (rigidbodyA != nullptr)
			desc.bodies[0].body = rigidbodyA->getRigidbody();

		ScriptRigidbody* rigidbodyB = bodiesArray.get<ScriptRigidbody*>(1);
		if (rigidbodyB != nullptr)
			desc.bodies[1].body = rigidbodyB->getRigidbody();

		ScriptArray positionArray(positions);
		desc.bodies[0].position = positionArray.get<Vector3>(0);
		desc.bodies[1].position = positionArray.get<Vector3>(1);

		ScriptArray rotationArray(rotations);
		desc.bodies[0].rotation = rotationArray.get<Quaternion>(0);
		desc.bodies[1].rotation = rotationArray.get<Quaternion>(1);

		desc.breakForce = breakForce;
		desc.breakTorque = breakTorque;
		desc.enableCollision = enableCollision;
	}
}