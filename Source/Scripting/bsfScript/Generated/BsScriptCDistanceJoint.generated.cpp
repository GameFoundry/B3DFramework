//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCDistanceJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCDistanceJoint.h"
#include "BsScriptSpring.generated.h"

namespace bs
{
	ScriptCDistanceJoint::ScriptCDistanceJoint(MonoObject* managedInstance, const GameObjectHandle<CDistanceJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCDistanceJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetDistance", (void*)&ScriptCDistanceJoint::InternalGetDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinDistance", (void*)&ScriptCDistanceJoint::InternalGetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetMinDistance", (void*)&ScriptCDistanceJoint::InternalSetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxDistance", (void*)&ScriptCDistanceJoint::InternalGetMaxDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxDistance", (void*)&ScriptCDistanceJoint::InternalSetMaxDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetTolerance", (void*)&ScriptCDistanceJoint::InternalGetTolerance);
		metaData.ScriptClass->AddInternalCall("Internal_SetTolerance", (void*)&ScriptCDistanceJoint::InternalSetTolerance);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpring", (void*)&ScriptCDistanceJoint::InternalGetSpring);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpring", (void*)&ScriptCDistanceJoint::InternalSetSpring);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptCDistanceJoint::InternalSetFlag);
		metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptCDistanceJoint::InternalHasFlag);

	}

	float ScriptCDistanceJoint::InternalGetDistance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptCDistanceJoint::InternalGetMinDistance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDistanceJoint::InternalSetMinDistance(ScriptCDistanceJoint* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetMinDistance(value);
	}

	float ScriptCDistanceJoint::InternalGetMaxDistance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDistanceJoint::InternalSetMaxDistance(ScriptCDistanceJoint* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetMaxDistance(value);
	}

	float ScriptCDistanceJoint::InternalGetTolerance(ScriptCDistanceJoint* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetTolerance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDistanceJoint::InternalSetTolerance(ScriptCDistanceJoint* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetTolerance(value);
	}

	void ScriptCDistanceJoint::InternalGetSpring(ScriptCDistanceJoint* thisPtr, Spring* __output)
	{
		Spring tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpring();

		*__output = tmp__output;
	}

	void ScriptCDistanceJoint::InternalSetSpring(ScriptCDistanceJoint* thisPtr, Spring* value)
	{
		thisPtr->GetHandle()->SetSpring(*value);
	}

	void ScriptCDistanceJoint::InternalSetFlag(ScriptCDistanceJoint* thisPtr, DistanceJointFlag flag, bool enabled)
	{
		thisPtr->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptCDistanceJoint::InternalHasFlag(ScriptCDistanceJoint* thisPtr, DistanceJointFlag flag)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
