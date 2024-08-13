//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCD6Joint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCD6Joint.h"
#include "BsScriptD6JointDrive.generated.h"
#include "BsScriptLimitLinear.generated.h"
#include "BsScriptLimitAngularRange.generated.h"
#include "BsScriptLimitConeRange.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptD6Joint::ScriptD6Joint(MonoObject* managedInstance, const GameObjectHandle<CD6Joint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptD6Joint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetMotion", (void*)&ScriptD6Joint::InternalGetMotion);
		metaData.ScriptClass->AddInternalCall("Internal_SetMotion", (void*)&ScriptD6Joint::InternalSetMotion);
		metaData.ScriptClass->AddInternalCall("Internal_GetTwist", (void*)&ScriptD6Joint::InternalGetTwist);
		metaData.ScriptClass->AddInternalCall("Internal_GetSwingY", (void*)&ScriptD6Joint::InternalGetSwingY);
		metaData.ScriptClass->AddInternalCall("Internal_GetSwingZ", (void*)&ScriptD6Joint::InternalGetSwingZ);
		metaData.ScriptClass->AddInternalCall("Internal_GetLimitLinear", (void*)&ScriptD6Joint::InternalGetLimitLinear);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimitLinear", (void*)&ScriptD6Joint::InternalSetLimitLinear);
		metaData.ScriptClass->AddInternalCall("Internal_GetLimitTwist", (void*)&ScriptD6Joint::InternalGetLimitTwist);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimitTwist", (void*)&ScriptD6Joint::InternalSetLimitTwist);
		metaData.ScriptClass->AddInternalCall("Internal_GetLimitSwing", (void*)&ScriptD6Joint::InternalGetLimitSwing);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimitSwing", (void*)&ScriptD6Joint::InternalSetLimitSwing);
		metaData.ScriptClass->AddInternalCall("Internal_GetDrive", (void*)&ScriptD6Joint::InternalGetDrive);
		metaData.ScriptClass->AddInternalCall("Internal_SetDrive", (void*)&ScriptD6Joint::InternalSetDrive);
		metaData.ScriptClass->AddInternalCall("Internal_GetDrivePosition", (void*)&ScriptD6Joint::InternalGetDrivePosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetDriveRotation", (void*)&ScriptD6Joint::InternalGetDriveRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetDriveTransform", (void*)&ScriptD6Joint::InternalSetDriveTransform);
		metaData.ScriptClass->AddInternalCall("Internal_GetDriveLinearVelocity", (void*)&ScriptD6Joint::InternalGetDriveLinearVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetDriveAngularVelocity", (void*)&ScriptD6Joint::InternalGetDriveAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetDriveVelocity", (void*)&ScriptD6Joint::InternalSetDriveVelocity);

	}

	D6JointMotion ScriptD6Joint::InternalGetMotion(ScriptD6Joint* self, D6JointAxis axis)
	{
		D6JointMotion tmp__output;
		tmp__output = self->GetHandle()->GetMotion(axis);

		D6JointMotion __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptD6Joint::InternalSetMotion(ScriptD6Joint* self, D6JointAxis axis, D6JointMotion motion)
	{
		self->GetHandle()->SetMotion(axis, motion);
	}

	void ScriptD6Joint::InternalGetTwist(ScriptD6Joint* self, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = self->GetHandle()->GetTwist();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetSwingY(ScriptD6Joint* self, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = self->GetHandle()->GetSwingY();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetSwingZ(ScriptD6Joint* self, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = self->GetHandle()->GetSwingZ();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetLimitLinear(ScriptD6Joint* self, __LimitLinearInterop* __output)
	{
		LimitLinear tmp__output;
		tmp__output = self->GetHandle()->GetLimitLinear();

		__LimitLinearInterop interop__output;
		interop__output = ScriptLimitLinear::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitLinear::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptD6Joint::InternalSetLimitLinear(ScriptD6Joint* self, __LimitLinearInterop* limit)
	{
		LimitLinear tmplimit;
		tmplimit = ScriptLimitLinear::FromInterop(*limit);
		self->GetHandle()->SetLimitLinear(tmplimit);
	}

	void ScriptD6Joint::InternalGetLimitTwist(ScriptD6Joint* self, __LimitAngularRangeInterop* __output)
	{
		LimitAngularRange tmp__output;
		tmp__output = self->GetHandle()->GetLimitTwist();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitAngularRange::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptD6Joint::InternalSetLimitTwist(ScriptD6Joint* self, __LimitAngularRangeInterop* limit)
	{
		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::FromInterop(*limit);
		self->GetHandle()->SetLimitTwist(tmplimit);
	}

	void ScriptD6Joint::InternalGetLimitSwing(ScriptD6Joint* self, __LimitConeRangeInterop* __output)
	{
		LimitConeRange tmp__output;
		tmp__output = self->GetHandle()->GetLimitSwing();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitConeRange::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptD6Joint::InternalSetLimitSwing(ScriptD6Joint* self, __LimitConeRangeInterop* limit)
	{
		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::FromInterop(*limit);
		self->GetHandle()->SetLimitSwing(tmplimit);
	}

	void ScriptD6Joint::InternalGetDrive(ScriptD6Joint* self, D6JointDriveType type, __D6JointDriveInterop* __output)
	{
		D6JointDrive tmp__output;
		tmp__output = self->GetHandle()->GetDrive(type);

		__D6JointDriveInterop interop__output;
		interop__output = ScriptD6JointDrive::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptD6JointDrive::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptD6Joint::InternalSetDrive(ScriptD6Joint* self, D6JointDriveType type, __D6JointDriveInterop* drive)
	{
		D6JointDrive tmpdrive;
		tmpdrive = ScriptD6JointDrive::FromInterop(*drive);
		self->GetHandle()->SetDrive(type, tmpdrive);
	}

	void ScriptD6Joint::InternalGetDrivePosition(ScriptD6Joint* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetDrivePosition();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetDriveRotation(ScriptD6Joint* self, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = self->GetHandle()->GetDriveRotation();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalSetDriveTransform(ScriptD6Joint* self, TVector3<float>* position, Quaternion* rotation)
	{
		self->GetHandle()->SetDriveTransform(*position, *rotation);
	}

	void ScriptD6Joint::InternalGetDriveLinearVelocity(ScriptD6Joint* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetDriveLinearVelocity();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetDriveAngularVelocity(ScriptD6Joint* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetDriveAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalSetDriveVelocity(ScriptD6Joint* self, TVector3<float>* linear, TVector3<float>* angular)
	{
		self->GetHandle()->SetDriveVelocity(*linear, *angular);
	}
}
