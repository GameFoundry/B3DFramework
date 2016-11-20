//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptJointCommon.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"

// DEBUG ONLY
#include "BsMonoProperty.h"

namespace bs
{
	ScriptD6JointDrive::GetNativeDataThunkDef ScriptD6JointDrive::getNativeDataThunk = nullptr;

	ScriptD6JointDrive::ScriptD6JointDrive(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptD6JointDrive::initRuntimeData()
	{
		getNativeDataThunk = (GetNativeDataThunkDef)metaData.scriptClass->getMethod("Internal_GetNative")->getThunk();
	}

	D6Joint::Drive ScriptD6JointDrive::convert(MonoObject* instance)
	{
		MonoException* exception = nullptr;
		MonoObject* boxedOutput = getNativeDataThunk(instance, &exception);

		MonoUtil::throwIfException(exception);
		return *(D6Joint::Drive*)MonoUtil::unbox(boxedOutput);
	}

	ScriptHingeJointDrive::GetNativeDataThunkDef ScriptHingeJointDrive::getNativeDataThunk = nullptr;

	ScriptHingeJointDrive::ScriptHingeJointDrive(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptHingeJointDrive::initRuntimeData()
	{
		getNativeDataThunk = (GetNativeDataThunkDef)metaData.scriptClass->getMethod("Internal_GetNative")->getThunk();
	}

	HingeJoint::Drive ScriptHingeJointDrive::convert(MonoObject* instance)
	{
		MonoException* exception = nullptr;
		MonoObject* boxedOutput = getNativeDataThunk(instance, &exception);

		MonoUtil::throwIfException(exception);
		return *(HingeJoint::Drive*)MonoUtil::unbox(boxedOutput);
	}

	ScriptLimitLinearRange::GetNativeDataThunkDef ScriptLimitLinearRange::getNativeDataThunk = nullptr;

	ScriptLimitLinearRange::ScriptLimitLinearRange(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptLimitLinearRange::initRuntimeData()
	{
		getNativeDataThunk = (GetNativeDataThunkDef)metaData.scriptClass->getMethod("Internal_GetNative")->getThunk();
	}

	LimitLinearRange ScriptLimitLinearRange::convert(MonoObject* instance)
	{
		MonoException* exception = nullptr;
		MonoObject* boxedOutput = getNativeDataThunk(instance, &exception);

		MonoUtil::throwIfException(exception);
		return *(LimitLinearRange*)MonoUtil::unbox(boxedOutput);
	}

	ScriptLimitLinear::GetNativeDataThunkDef ScriptLimitLinear::getNativeDataThunk = nullptr;

	ScriptLimitLinear::ScriptLimitLinear(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptLimitLinear::initRuntimeData()
	{
		getNativeDataThunk = (GetNativeDataThunkDef)metaData.scriptClass->getMethod("Internal_GetNative")->getThunk();
	}

	LimitLinear ScriptLimitLinear::convert(MonoObject* instance)
	{
		MonoException* exception = nullptr;
		MonoObject* boxedOutput = getNativeDataThunk(instance, &exception);

		MonoUtil::throwIfException(exception);
		return *(LimitLinear*)MonoUtil::unbox(boxedOutput);
	}

	ScriptLimitAngularRange::GetNativeDataThunkDef ScriptLimitAngularRange::getNativeDataThunk = nullptr;

	ScriptLimitAngularRange::ScriptLimitAngularRange(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptLimitAngularRange::initRuntimeData()
	{
		getNativeDataThunk = (GetNativeDataThunkDef)metaData.scriptClass->getMethod("Internal_GetNative")->getThunk();
	}

	LimitAngularRange ScriptLimitAngularRange::convert(MonoObject* instance)
	{
		MonoException* exception = nullptr;
		MonoObject* boxedOutput = getNativeDataThunk(instance, &exception);

		MonoUtil::throwIfException(exception);
		return *(LimitAngularRange*)MonoUtil::unbox(boxedOutput);
	}

	ScriptLimitConeRange::GetNativeDataThunkDef ScriptLimitConeRange::getNativeDataThunk = nullptr;

	ScriptLimitConeRange::ScriptLimitConeRange(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptLimitConeRange::initRuntimeData()
	{
		getNativeDataThunk = (GetNativeDataThunkDef)metaData.scriptClass->getMethod("Internal_GetNative")->getThunk();
	}

	LimitConeRange ScriptLimitConeRange::convert(MonoObject* instance)
	{
		MonoException* exception = nullptr;
		MonoObject* boxedOutput = getNativeDataThunk(instance, &exception);

		MonoUtil::throwIfException(exception);
		return *(LimitConeRange*)MonoUtil::unbox(boxedOutput);
	}
}