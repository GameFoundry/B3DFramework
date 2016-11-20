//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptFixedJoint.h"
#include "BsFixedJoint.h"

namespace bs
{
	ScriptFixedJoint::ScriptFixedJoint(MonoObject* instance, const SPtr<Joint>& joint)
		:TScriptJoint(instance, joint)
	{

	}

	void ScriptFixedJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptFixedJoint::internal_CreateInstance);
	}

	FixedJoint* ScriptFixedJoint::getFixedJoint() const
	{
		return static_cast<FixedJoint*>(mJoint.get());
	}

	void ScriptFixedJoint::internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData)
	{
		FIXED_JOINT_DESC desc;
		commonData->toDesc(desc);

		SPtr<FixedJoint> joint = FixedJoint::create(desc);
		joint->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptFixedJoint>()) ScriptFixedJoint(instance, joint);
	}
}