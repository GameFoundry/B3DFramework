//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCD6Joint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCD6Joint.h"
#include "BsScriptD6JointDrive.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptLimitLinear.generated.h"
#include "BsScriptLimitAngularRange.generated.h"
#include "BsScriptLimitConeRange.generated.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptCD6Joint::ScriptCD6Joint(MonoObject* managedInstance, const GameObjectHandle<CD6Joint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCD6Joint::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getMotion", (void*)&ScriptCD6Joint::Internal_getMotion);
		metaData.scriptClass->AddInternalCall("Internal_setMotion", (void*)&ScriptCD6Joint::Internal_setMotion);
		metaData.scriptClass->AddInternalCall("Internal_getTwist", (void*)&ScriptCD6Joint::Internal_getTwist);
		metaData.scriptClass->AddInternalCall("Internal_getSwingY", (void*)&ScriptCD6Joint::Internal_getSwingY);
		metaData.scriptClass->AddInternalCall("Internal_getSwingZ", (void*)&ScriptCD6Joint::Internal_getSwingZ);
		metaData.scriptClass->AddInternalCall("Internal_getLimitLinear", (void*)&ScriptCD6Joint::Internal_getLimitLinear);
		metaData.scriptClass->AddInternalCall("Internal_setLimitLinear", (void*)&ScriptCD6Joint::Internal_setLimitLinear);
		metaData.scriptClass->AddInternalCall("Internal_getLimitTwist", (void*)&ScriptCD6Joint::Internal_getLimitTwist);
		metaData.scriptClass->AddInternalCall("Internal_setLimitTwist", (void*)&ScriptCD6Joint::Internal_setLimitTwist);
		metaData.scriptClass->AddInternalCall("Internal_getLimitSwing", (void*)&ScriptCD6Joint::Internal_getLimitSwing);
		metaData.scriptClass->AddInternalCall("Internal_setLimitSwing", (void*)&ScriptCD6Joint::Internal_setLimitSwing);
		metaData.scriptClass->AddInternalCall("Internal_getDrive", (void*)&ScriptCD6Joint::Internal_getDrive);
		metaData.scriptClass->AddInternalCall("Internal_setDrive", (void*)&ScriptCD6Joint::Internal_setDrive);
		metaData.scriptClass->AddInternalCall("Internal_getDrivePosition", (void*)&ScriptCD6Joint::Internal_getDrivePosition);
		metaData.scriptClass->AddInternalCall("Internal_getDriveRotation", (void*)&ScriptCD6Joint::Internal_getDriveRotation);
		metaData.scriptClass->AddInternalCall("Internal_setDriveTransform", (void*)&ScriptCD6Joint::Internal_setDriveTransform);
		metaData.scriptClass->AddInternalCall("Internal_getDriveLinearVelocity", (void*)&ScriptCD6Joint::Internal_getDriveLinearVelocity);
		metaData.scriptClass->AddInternalCall("Internal_getDriveAngularVelocity", (void*)&ScriptCD6Joint::Internal_getDriveAngularVelocity);
		metaData.scriptClass->AddInternalCall("Internal_setDriveVelocity", (void*)&ScriptCD6Joint::Internal_setDriveVelocity);

	}

	D6JointMotion ScriptCD6Joint::Internal_getMotion(ScriptCD6Joint* thisPtr, D6JointAxis axis)
	{
		D6JointMotion tmp__output;
		tmp__output = thisPtr->GetHandle()->getMotion(axis);

		D6JointMotion __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCD6Joint::Internal_setMotion(ScriptCD6Joint* thisPtr, D6JointAxis axis, D6JointMotion motion)
	{
		thisPtr->GetHandle()->setMotion(axis, motion);
	}

	void ScriptCD6Joint::Internal_getTwist(ScriptCD6Joint* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->GetHandle()->getTwist();

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_getSwingY(ScriptCD6Joint* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->GetHandle()->getSwingY();

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_getSwingZ(ScriptCD6Joint* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->GetHandle()->getSwingZ();

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_getLimitLinear(ScriptCD6Joint* thisPtr, __LimitLinearInterop* __output)
	{
		LimitLinear tmp__output;
		tmp__output = thisPtr->GetHandle()->getLimitLinear();

		__LimitLinearInterop interop__output;
		interop__output = ScriptLimitLinear::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitLinear::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptCD6Joint::Internal_setLimitLinear(ScriptCD6Joint* thisPtr, __LimitLinearInterop* limit)
	{
		LimitLinear tmplimit;
		tmplimit = ScriptLimitLinear::fromInterop(*limit);
		thisPtr->GetHandle()->setLimitLinear(tmplimit);
	}

	void ScriptCD6Joint::Internal_getLimitTwist(ScriptCD6Joint* thisPtr, __LimitAngularRangeInterop* __output)
	{
		LimitAngularRange tmp__output;
		tmp__output = thisPtr->GetHandle()->getLimitTwist();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitAngularRange::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptCD6Joint::Internal_setLimitTwist(ScriptCD6Joint* thisPtr, __LimitAngularRangeInterop* limit)
	{
		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::fromInterop(*limit);
		thisPtr->GetHandle()->setLimitTwist(tmplimit);
	}

	void ScriptCD6Joint::Internal_getLimitSwing(ScriptCD6Joint* thisPtr, __LimitConeRangeInterop* __output)
	{
		LimitConeRange tmp__output;
		tmp__output = thisPtr->GetHandle()->getLimitSwing();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptLimitConeRange::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptCD6Joint::Internal_setLimitSwing(ScriptCD6Joint* thisPtr, __LimitConeRangeInterop* limit)
	{
		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::fromInterop(*limit);
		thisPtr->GetHandle()->setLimitSwing(tmplimit);
	}

	void ScriptCD6Joint::Internal_getDrive(ScriptCD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* __output)
	{
		D6JointDrive tmp__output;
		tmp__output = thisPtr->GetHandle()->getDrive(type);

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_setDrive(ScriptCD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* drive)
	{
		thisPtr->GetHandle()->setDrive(type, *drive);
	}

	void ScriptCD6Joint::Internal_getDrivePosition(ScriptCD6Joint* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getDrivePosition();

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_getDriveRotation(ScriptCD6Joint* thisPtr, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetHandle()->getDriveRotation();

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_setDriveTransform(ScriptCD6Joint* thisPtr, Vector3* position, Quaternion* rotation)
	{
		thisPtr->GetHandle()->setDriveTransform(*position, *rotation);
	}

	void ScriptCD6Joint::Internal_getDriveLinearVelocity(ScriptCD6Joint* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getDriveLinearVelocity();

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_getDriveAngularVelocity(ScriptCD6Joint* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getDriveAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptCD6Joint::Internal_setDriveVelocity(ScriptCD6Joint* thisPtr, Vector3* linear, Vector3* angular)
	{
		thisPtr->GetHandle()->setDriveVelocity(*linear, *angular);
	}
}
