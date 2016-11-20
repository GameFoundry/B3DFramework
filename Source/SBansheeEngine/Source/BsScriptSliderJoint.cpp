//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSliderJoint.h"
#include "BsScriptJointCommon.h"
#include "BsSliderJoint.h"

namespace bs
{
	ScriptSliderJoint::ScriptSliderJoint(MonoObject* instance, const SPtr<Joint>& joint)
		:TScriptJoint(instance, joint)
	{

	}

	void ScriptSliderJoint::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptSliderJoint::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPosition", &ScriptSliderJoint::internal_GetPosition);
		metaData.scriptClass->addInternalCall("Internal_GetSpeed", &ScriptSliderJoint::internal_GetSpeed);
		metaData.scriptClass->addInternalCall("Internal_SetLimit", &ScriptSliderJoint::internal_SetLimit);
		metaData.scriptClass->addInternalCall("Internal_SetEnableLimit", &ScriptSliderJoint::internal_SetEnableLimit);
	}

	SliderJoint* ScriptSliderJoint::getSliderJoint() const
	{
		return static_cast<SliderJoint*>(mJoint.get());
	}

	void ScriptSliderJoint::internal_CreateInstance(MonoObject* instance, ScriptCommonJointData* commonData, ScriptSliderJointData* data)
	{
		SLIDER_JOINT_DESC desc;
		commonData->toDesc(desc);
		data->toDesc(desc);

		SPtr<SliderJoint> joint = SliderJoint::create(desc);
		joint->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptSliderJoint>()) ScriptSliderJoint(instance, joint);
	}

	float ScriptSliderJoint::internal_GetPosition(ScriptSliderJoint* thisPtr)
	{
		return thisPtr->getSliderJoint()->getPosition();
	}

	float ScriptSliderJoint::internal_GetSpeed(ScriptSliderJoint* thisPtr)
	{
		return thisPtr->getSliderJoint()->getSpeed();
	}

	void ScriptSliderJoint::internal_SetLimit(ScriptSliderJoint* thisPtr, MonoObject* limit)
	{
		LimitLinearRange nativeLimit = ScriptLimitLinearRange::convert(limit);
		thisPtr->getSliderJoint()->setLimit(nativeLimit);
	}

	void ScriptSliderJoint::internal_SetEnableLimit(ScriptSliderJoint* thisPtr, bool enable)
	{
		thisPtr->getSliderJoint()->setFlag(SliderJoint::Flag::Limit, enable);
	}

	void ScriptSliderJointData::toDesc(SLIDER_JOINT_DESC& desc) const
	{
		desc.limit = ScriptLimitLinearRange::convert(limit);
		desc.flag = enableLimit ? SliderJoint::Flag::Limit : (SliderJoint::Flag)0;
	}
}