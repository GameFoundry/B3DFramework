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
	ScriptDistanceJoint::ScriptDistanceJoint(MonoObject* managedInstance, const GameObjectHandle<CDistanceJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptDistanceJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetDistance", (void*)&ScriptDistanceJoint::InternalGetDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinDistance", (void*)&ScriptDistanceJoint::InternalGetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetMinDistance", (void*)&ScriptDistanceJoint::InternalSetMinDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxDistance", (void*)&ScriptDistanceJoint::InternalGetMaxDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxDistance", (void*)&ScriptDistanceJoint::InternalSetMaxDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetTolerance", (void*)&ScriptDistanceJoint::InternalGetTolerance);
		metaData.ScriptClass->AddInternalCall("Internal_SetTolerance", (void*)&ScriptDistanceJoint::InternalSetTolerance);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpring", (void*)&ScriptDistanceJoint::InternalGetSpring);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpring", (void*)&ScriptDistanceJoint::InternalSetSpring);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptDistanceJoint::InternalSetFlag);
		metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptDistanceJoint::InternalHasFlag);

	}

	float ScriptDistanceJoint::InternalGetDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptDistanceJoint::InternalGetMinDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetMinDistance(ScriptDistanceJoint* self, float value)
	{
		self->GetHandle()->SetMinDistance(value);
	}

	float ScriptDistanceJoint::InternalGetMaxDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetMaxDistance(ScriptDistanceJoint* self, float value)
	{
		self->GetHandle()->SetMaxDistance(value);
	}

	float ScriptDistanceJoint::InternalGetTolerance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetTolerance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetTolerance(ScriptDistanceJoint* self, float value)
	{
		self->GetHandle()->SetTolerance(value);
	}

	void ScriptDistanceJoint::InternalGetSpring(ScriptDistanceJoint* self, Spring* __output)
	{
		Spring tmp__output;
		tmp__output = self->GetHandle()->GetSpring();

		*__output = tmp__output;
	}

	void ScriptDistanceJoint::InternalSetSpring(ScriptDistanceJoint* self, Spring* value)
	{
		self->GetHandle()->SetSpring(*value);
	}

	void ScriptDistanceJoint::InternalSetFlag(ScriptDistanceJoint* self, DistanceJointFlag flag, bool enabled)
	{
		self->GetHandle()->SetFlag(flag, enabled);
	}

	bool ScriptDistanceJoint::InternalHasFlag(ScriptDistanceJoint* self, DistanceJointFlag flag)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
