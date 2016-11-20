//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptDistanceJoint.h"
#include "BsDistanceJoint.h"

namespace bs
{
	ScriptDistanceJoint::ScriptDistanceJoint(MonoObject* instance, const SPtr<Joint>& joint)
		:TScriptJoint(instance, joint)
	{

	}

	void ScriptDistanceJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptDistanceJoint::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetDistance", &ScriptDistanceJoint::internal_GetDistance);
		metaData.scriptClass->addInternalCall("Internal_SetMinDistance", &ScriptDistanceJoint::internal_SetMinDistance);
		metaData.scriptClass->addInternalCall("Internal_SetMaxDistance", &ScriptDistanceJoint::internal_SetMaxDistance);
		metaData.scriptClass->addInternalCall("Internal_SetTolerance", &ScriptDistanceJoint::internal_SetTolerance);
		metaData.scriptClass->addInternalCall("Internal_SetSpring", &ScriptDistanceJoint::internal_SetSpring);
		metaData.scriptClass->addInternalCall("Internal_SetEnableMinDistanceLimit", &ScriptDistanceJoint::internal_SetEnableMinDistanceLimit);
		metaData.scriptClass->addInternalCall("Internal_SetEnableMaxDistanceLimit", &ScriptDistanceJoint::internal_SetEnableMaxDistanceLimit);
		metaData.scriptClass->addInternalCall("Internal_SetEnableSpring", &ScriptDistanceJoint::internal_SetEnableSpring);
	}

	DistanceJoint* ScriptDistanceJoint::getDistanceJoint() const
	{
		return static_cast<DistanceJoint*>(mJoint.get());
	}

	void ScriptDistanceJoint::internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData,
		ScriptDistanceJointData* data)
	{
		DISTANCE_JOINT_DESC desc;
		commonData->toDesc(desc);
		data->toDesc(desc);

		SPtr<DistanceJoint> joint = DistanceJoint::create(desc);
		joint->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptDistanceJoint>()) ScriptDistanceJoint(instance, joint);
	}

	float ScriptDistanceJoint::internal_GetDistance(ScriptDistanceJoint* thisPtr)
	{
		return thisPtr->getDistanceJoint()->getDistance();
	}

	void ScriptDistanceJoint::internal_SetMinDistance(ScriptDistanceJoint* thisPtr, float value)
	{
		thisPtr->getDistanceJoint()->setMinDistance(value);
	}

	void ScriptDistanceJoint::internal_SetMaxDistance(ScriptDistanceJoint* thisPtr, float value)
	{
		thisPtr->getDistanceJoint()->setMaxDistance(value);
	}

	void ScriptDistanceJoint::internal_SetTolerance(ScriptDistanceJoint* thisPtr, float value)
	{
		thisPtr->getDistanceJoint()->setTolerance(value);
	}

	void ScriptDistanceJoint::internal_SetSpring(ScriptDistanceJoint* thisPtr, Spring* value)
	{
		thisPtr->getDistanceJoint()->setSpring(*value);
	}

	void ScriptDistanceJoint::internal_SetEnableMinDistanceLimit(ScriptDistanceJoint* thisPtr, bool value)
	{
		thisPtr->getDistanceJoint()->setFlag(DistanceJoint::Flag::MinDistance, value);
	}

	void ScriptDistanceJoint::internal_SetEnableMaxDistanceLimit(ScriptDistanceJoint* thisPtr, bool value)
	{
		thisPtr->getDistanceJoint()->setFlag(DistanceJoint::Flag::MaxDistance, value);
	}

	void ScriptDistanceJoint::internal_SetEnableSpring(ScriptDistanceJoint* thisPtr, bool value)
	{
		thisPtr->getDistanceJoint()->setFlag(DistanceJoint::Flag::Spring, value);
	}

	void ScriptDistanceJointData::toDesc(DISTANCE_JOINT_DESC& desc) const
	{
		desc.minDistance = minDistance;
		desc.maxDistance = maxDistance;
		desc.tolerance = tolerance;
		desc.spring = spring;
		desc.flag = flags;
	}
}