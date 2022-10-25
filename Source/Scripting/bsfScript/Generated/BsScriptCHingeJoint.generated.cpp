//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCHingeJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCHingeJoint.h"
#include "BsScriptHingeJointDrive.generated.h"
#include "BsScriptLimitAngularRange.generated.h"

namespace bs
{
	ScriptCHingeJoint::ScriptCHingeJoint(MonoObject* managedInstance, const GameObjectHandle<CHingeJoint>& value)
		: TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCHingeJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetAngle", (void*)&ScriptCHingeJoint::InternalGetAngle);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptCHingeJoint::InternalGetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptCHingeJoint::InternalGetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptCHingeJoint::InternalSetLimit);
		metaData.ScriptClass->AddInternalCall("Internal_GetDrive", (void*)&ScriptCHingeJoint::InternalGetDrive);
		metaData.ScriptClass->AddInternalCall("Internal_SetDrive", (void*)&ScriptCHingeJoint::InternalSetDrive);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptCHingeJoint::InternalSetFlag);
		metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptCHingeJoint::InternalHasFlag);
	}

	void ScriptCHingeJoint::InternalGetAngle(ScriptCHingeJoint* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAngle();

		*__output = tmp__output;
	}

	float ScriptCHingeJoint::InternalGetSpeed(ScriptCHingeJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCHingeJoint::InternalGetLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* __output)
	{
		LimitAngularRange tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLimit();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitAngularRange::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptCHingeJoint::InternalSetLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* limit)
	{
		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::FromInterop(*limit);
		thisPtr->GetHandle()->SetLimit(tmplimit);
	}

	void ScriptCHingeJoint::InternalGetDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* __output)
	{
		HingeJointDrive tmp__output;
		tmp__output = thisPtr->GetHandle()->GetDrive();

		*__output = tmp__output;
	}

	void ScriptCHingeJoint::InternalSetDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* drive)
	{
		thisPtr->GetHandle()->SetDrive(*drive);
	}

	void ScriptCHingeJoint::InternalSetFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag, bool enabled)
	{
		thisPtr->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptCHingeJoint::InternalHasFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
} // namespace bs
