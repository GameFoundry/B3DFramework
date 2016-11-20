//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSphericalJoint.h"
#include "BsScriptJointCommon.h"
#include "BsSphericalJoint.h"

namespace bs
{
	ScriptSphericalJoint::ScriptSphericalJoint(MonoObject* instance, const SPtr<Joint>& joint)
		:TScriptJoint(instance, joint)
	{

	}

	void ScriptSphericalJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptSphericalJoint::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_SetLimit", &ScriptSphericalJoint::internal_SetLimit);
		metaData.scriptClass->addInternalCall("Internal_SetEnableLimit", &ScriptSphericalJoint::internal_SetEnableLimit);
	}

	SphericalJoint* ScriptSphericalJoint::getSphericalJoint() const
	{
		return static_cast<SphericalJoint*>(mJoint.get());
	}

	void ScriptSphericalJoint::internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData, ScriptSphericalJointData* data)
	{
		SPHERICAL_JOINT_DESC desc;
		commonData->toDesc(desc);
		data->toDesc(desc);

		SPtr<SphericalJoint> joint = SphericalJoint::create(desc);
		joint->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptSphericalJoint>()) ScriptSphericalJoint(instance, joint);
	}

	void ScriptSphericalJoint::internal_SetLimit(ScriptSphericalJoint* thisPtr, MonoObject* limit)
	{
		LimitConeRange nativeLimit = ScriptLimitConeRange::convert(limit);
		thisPtr->getSphericalJoint()->setLimit(nativeLimit);
	}

	void ScriptSphericalJoint::internal_SetEnableLimit(ScriptSphericalJoint* thisPtr, bool enable)
	{
		thisPtr->getSphericalJoint()->setFlag(SphericalJoint::Flag::Limit, enable);
	}

	void ScriptSphericalJointData::toDesc(SPHERICAL_JOINT_DESC& desc) const
	{
		desc.limit = ScriptLimitConeRange::convert(limit);
		desc.flag = enableLimit ? SphericalJoint::Flag::Limit : (SphericalJoint::Flag)0;
	}
}