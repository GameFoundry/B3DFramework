//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptHingeJoint.h"
#include "BsHingeJoint.h"
#include "BsScriptJointCommon.h"

namespace bs
{
	ScriptHingeJoint::ScriptHingeJoint(MonoObject* instance, const SPtr<Joint>& joint)
		:TScriptJoint(instance, joint)
	{

	}

	void ScriptHingeJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptHingeJoint::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetAngle", &ScriptHingeJoint::internal_GetAngle);
		metaData.scriptClass->addInternalCall("Internal_GetSpeed", &ScriptHingeJoint::internal_GetSpeed);
		metaData.scriptClass->addInternalCall("Internal_SetLimit", &ScriptHingeJoint::internal_SetLimit);
		metaData.scriptClass->addInternalCall("Internal_SetDrive", &ScriptHingeJoint::internal_SetDrive);
		metaData.scriptClass->addInternalCall("Internal_SetEnableLimit", &ScriptHingeJoint::internal_SetEnableLimit);
		metaData.scriptClass->addInternalCall("Internal_SetEnableDrive", &ScriptHingeJoint::internal_SetEnableDrive);
	}

	HingeJoint* ScriptHingeJoint::getHingeJoint() const
	{
		return static_cast<HingeJoint*>(mJoint.get());
	}

	void ScriptHingeJoint::internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData, ScriptHingeJointData* data)
	{
		HINGE_JOINT_DESC desc;
		commonData->toDesc(desc);
		data->toDesc(desc);

		SPtr<HingeJoint> joint = HingeJoint::create(desc);
		joint->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptHingeJoint>()) ScriptHingeJoint(instance, joint);
	}

	float ScriptHingeJoint::internal_GetAngle(ScriptHingeJoint* thisPtr)
	{
		return thisPtr->getHingeJoint()->getAngle().valueRadians();
	}

	float ScriptHingeJoint::internal_GetSpeed(ScriptHingeJoint* thisPtr)
	{
		return thisPtr->getHingeJoint()->getSpeed();
	}

	void ScriptHingeJoint::internal_SetLimit(ScriptHingeJoint* thisPtr, MonoObject* limit)
	{
		LimitAngularRange nativeLimit = ScriptLimitAngularRange::convert(limit);

		thisPtr->getHingeJoint()->setLimit(nativeLimit);
	}

	void ScriptHingeJoint::internal_SetDrive(ScriptHingeJoint* thisPtr, MonoObject* drive)
	{
		HingeJoint::Drive nativeDrive = ScriptHingeJointDrive::convert(drive);
		thisPtr->getHingeJoint()->setDrive(nativeDrive);
	}

	void ScriptHingeJoint::internal_SetEnableLimit(ScriptHingeJoint* thisPtr, bool value)
	{
		thisPtr->getHingeJoint()->setFlag(HingeJoint::Flag::Limit, value);
	}

	void ScriptHingeJoint::internal_SetEnableDrive(ScriptHingeJoint* thisPtr, bool value)
	{
		thisPtr->getHingeJoint()->setFlag(HingeJoint::Flag::Drive, value);
	}

	void ScriptHingeJointData::toDesc(HINGE_JOINT_DESC& desc) const
	{
		desc.limit = ScriptLimitAngularRange::convert(limit);
		desc.drive = ScriptHingeJointDrive::convert(drive);
		desc.flag = flags;
	}
}