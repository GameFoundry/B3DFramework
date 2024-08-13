//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCHingeJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCHingeJoint.h"
#include "BsScriptLimitAngularRange.generated.h"
#include "BsScriptHingeJointDrive.generated.h"

namespace bs
{
	ScriptHingeJoint::ScriptHingeJoint(MonoObject* managedInstance, const GameObjectHandle<CHingeJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptHingeJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetAngle", (void*)&ScriptHingeJoint::InternalGetAngle);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptHingeJoint::InternalGetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptHingeJoint::InternalGetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptHingeJoint::InternalSetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_GetDrive", (void*)&ScriptHingeJoint::InternalGetDrive);
		metaData.ScriptClass->AddInternalCall("Internal_SetDrive", (void*)&ScriptHingeJoint::InternalSetDrive);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptHingeJoint::InternalSetFlag);
		metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptHingeJoint::InternalHasFlag);

	}

	void ScriptHingeJoint::InternalGetAngle(ScriptHingeJoint* self, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = self->GetHandle()->GetAngle();

		*__output = tmp__output;
	}

	float ScriptHingeJoint::InternalGetSpeed(ScriptHingeJoint* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptHingeJoint::InternalGetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* __output)
	{
		LimitAngularRange tmp__output;
		tmp__output = self->GetHandle()->GetLimit();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitAngularRange::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptHingeJoint::InternalSetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* limit)
	{
		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::FromInterop(*limit);
		self->GetHandle()->SetLimit(tmplimit);
	}

	void ScriptHingeJoint::InternalGetDrive(ScriptHingeJoint* self, HingeJointDrive* __output)
	{
		HingeJointDrive tmp__output;
		tmp__output = self->GetHandle()->GetDrive();

		*__output = tmp__output;
	}

	void ScriptHingeJoint::InternalSetDrive(ScriptHingeJoint* self, HingeJointDrive* drive)
	{
		self->GetHandle()->SetDrive(*drive);
	}

	void ScriptHingeJoint::InternalSetFlag(ScriptHingeJoint* self, HingeJointFlag flag, bool enabled)
	{
		self->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptHingeJoint::InternalHasFlag(ScriptHingeJoint* self, HingeJointFlag flag)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
