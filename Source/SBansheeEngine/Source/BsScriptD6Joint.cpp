//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptD6Joint.h"
#include "BsScriptJointCommon.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptD6Joint::ScriptD6Joint(MonoObject* instance, const SPtr<Joint>& joint)
		:TScriptJoint(instance, joint)
	{

	}

	void ScriptD6Joint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptD6Joint::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_SetMotion", &ScriptD6Joint::internal_SetMotion);
		metaData.scriptClass->addInternalCall("Internal_GetTwist", &ScriptD6Joint::internal_GetTwist);
		metaData.scriptClass->addInternalCall("Internal_GetSwingY", &ScriptD6Joint::internal_GetSwingY);
		metaData.scriptClass->addInternalCall("Internal_GetSwingZ", &ScriptD6Joint::internal_GetSwingZ);
		metaData.scriptClass->addInternalCall("Internal_SetLimitLinear", &ScriptD6Joint::internal_SetLimitLinear);
		metaData.scriptClass->addInternalCall("Internal_SetLimitTwist", &ScriptD6Joint::internal_SetLimitTwist);
		metaData.scriptClass->addInternalCall("Internal_SetLimitSwing", &ScriptD6Joint::internal_SetLimitSwing);
		metaData.scriptClass->addInternalCall("Internal_SetDrive", &ScriptD6Joint::internal_SetDrive);
		metaData.scriptClass->addInternalCall("Internal_SetDrivePosition", &ScriptD6Joint::internal_SetDrivePosition);
		metaData.scriptClass->addInternalCall("Internal_SetDriveRotation", &ScriptD6Joint::internal_SetDriveRotation);
		metaData.scriptClass->addInternalCall("Internal_SetDriveLinearVelocity", &ScriptD6Joint::internal_SetDriveLinearVelocity);
		metaData.scriptClass->addInternalCall("Internal_SetDriveAngularVelocity", &ScriptD6Joint::internal_SetDriveAngularVelocity);
	}

	D6Joint* ScriptD6Joint::getD6Joint() const
	{
		return static_cast<D6Joint*>(mJoint.get());
	}

	void ScriptD6Joint::internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData, ScriptD6JointData* data)
	{
		D6_JOINT_DESC desc;
		commonData->toDesc(desc);
		data->toDesc(desc);

		SPtr<D6Joint> joint = D6Joint::create(desc);
		joint->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptD6Joint>()) ScriptD6Joint(instance, joint);
	}

	void ScriptD6Joint::internal_SetMotion(ScriptD6Joint* thisPtr, D6Joint::Axis axis, D6Joint::Motion motion)
	{
		thisPtr->getD6Joint()->setMotion(axis, motion);
	}

	float ScriptD6Joint::internal_GetTwist(ScriptD6Joint* thisPtr)
	{
		return thisPtr->getD6Joint()->getTwist().valueRadians();
	}

	float ScriptD6Joint::internal_GetSwingY(ScriptD6Joint* thisPtr)
	{
		return thisPtr->getD6Joint()->getSwingY().valueRadians();
	}

	float ScriptD6Joint::internal_GetSwingZ(ScriptD6Joint* thisPtr)
	{
		return thisPtr->getD6Joint()->getSwingZ().valueRadians();
	}

	void ScriptD6Joint::internal_SetLimitLinear(ScriptD6Joint* thisPtr, MonoObject* limit)
	{
		LimitLinear nativeLimit = ScriptLimitLinear::convert(limit);
		thisPtr->getD6Joint()->setLimitLinear(nativeLimit);
	}

	void ScriptD6Joint::internal_SetLimitTwist(ScriptD6Joint* thisPtr, MonoObject* limit)
	{
		LimitAngularRange nativeLimit = ScriptLimitAngularRange::convert(limit);
		thisPtr->getD6Joint()->setLimitTwist(nativeLimit);
	}

	void ScriptD6Joint::internal_SetLimitSwing(ScriptD6Joint* thisPtr, MonoObject* limit)
	{
		LimitConeRange nativeLimit = ScriptLimitConeRange::convert(limit);
		thisPtr->getD6Joint()->setLimitSwing(nativeLimit);
	}

	void ScriptD6Joint::internal_SetDrive(ScriptD6Joint* thisPtr, D6Joint::DriveType type, MonoObject* drive)
	{
		D6Joint::Drive nativeDrive = ScriptD6JointDrive::convert(drive);
		thisPtr->getD6Joint()->setDrive(type, nativeDrive);
	}

	void ScriptD6Joint::internal_SetDrivePosition(ScriptD6Joint* thisPtr, Vector3* position)
	{
		Quaternion rotation = thisPtr->getD6Joint()->getDriveRotation();
		thisPtr->getD6Joint()->setDriveTransform(*position, rotation);
	}

	void ScriptD6Joint::internal_SetDriveRotation(ScriptD6Joint* thisPtr, Quaternion* rotation)
	{
		Vector3 position = thisPtr->getD6Joint()->getDrivePosition();
		thisPtr->getD6Joint()->setDriveTransform(position, *rotation);
	}

	void ScriptD6Joint::internal_SetDriveLinearVelocity(ScriptD6Joint* thisPtr, Vector3* velocity)
	{
		Vector3 angularVelocity = thisPtr->getD6Joint()->getDriveAngularVelocity();
		thisPtr->getD6Joint()->setDriveVelocity(*velocity, angularVelocity);
	}

	void ScriptD6Joint::internal_SetDriveAngularVelocity(ScriptD6Joint* thisPtr, Vector3* velocity)
	{
		Vector3 linearVelocity = thisPtr->getD6Joint()->getDriveLinearVelocity();
		thisPtr->getD6Joint()->setDriveVelocity(linearVelocity, *velocity);
	}

	void ScriptD6JointData::toDesc(D6_JOINT_DESC& desc) const
	{
		ScriptArray motionArray(motion);
		for (UINT32 i = 0; i < (UINT32)D6Joint::Axis::Count; i++)
			desc.motion[i] = motionArray.get<D6Joint::Motion>(i);

		ScriptArray driveArray(drives);
		for (UINT32 i = 0; i < (UINT32)D6Joint::DriveType::Count; i++)
			desc.drive[i] = ScriptD6JointDrive::convert(driveArray.get<MonoObject*>(i));

		desc.limitLinear = ScriptLimitLinear::convert(linearLimit);
		desc.limitTwist = ScriptLimitAngularRange::convert(twistLimit);
		desc.limitSwing = ScriptLimitConeRange::convert(swingLimit);
		desc.drivePosition = drivePosition;
		desc.driveRotation = driveRotation;
		desc.driveLinearVelocity = driveLinearVelocity;
		desc.driveAngularVelocity = driveAngularVelocity;
	}
}