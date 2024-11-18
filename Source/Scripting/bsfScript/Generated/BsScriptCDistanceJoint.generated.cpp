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
	ScriptDistanceJoint::ScriptDistanceJoint(const GameObjectHandle<CDistanceJoint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptDistanceJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDistance", (void*)&ScriptDistanceJoint::InternalGetDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinDistance", (void*)&ScriptDistanceJoint::InternalGetMinDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMinDistance", (void*)&ScriptDistanceJoint::InternalSetMinDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxDistance", (void*)&ScriptDistanceJoint::InternalGetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxDistance", (void*)&ScriptDistanceJoint::InternalSetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTolerance", (void*)&ScriptDistanceJoint::InternalGetTolerance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTolerance", (void*)&ScriptDistanceJoint::InternalSetTolerance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpring", (void*)&ScriptDistanceJoint::InternalGetSpring);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpring", (void*)&ScriptDistanceJoint::InternalSetSpring);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptDistanceJoint::InternalSetFlag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptDistanceJoint::InternalHasFlag);

	}

	MonoObject* ScriptDistanceJoint::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	float ScriptDistanceJoint::InternalGetDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CDistanceJoint*>(self->GetNativeObject())->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptDistanceJoint::InternalGetMinDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CDistanceJoint*>(self->GetNativeObject())->GetMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetMinDistance(ScriptDistanceJoint* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CDistanceJoint*>(self->GetNativeObject())->SetMinDistance(value);
	}

	float ScriptDistanceJoint::InternalGetMaxDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CDistanceJoint*>(self->GetNativeObject())->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetMaxDistance(ScriptDistanceJoint* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CDistanceJoint*>(self->GetNativeObject())->SetMaxDistance(value);
	}

	float ScriptDistanceJoint::InternalGetTolerance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CDistanceJoint*>(self->GetNativeObject())->GetTolerance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetTolerance(ScriptDistanceJoint* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CDistanceJoint*>(self->GetNativeObject())->SetTolerance(value);
	}

	void ScriptDistanceJoint::InternalGetSpring(ScriptDistanceJoint* self, Spring* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		Spring tmp__output;
		tmp__output = static_cast<CDistanceJoint*>(self->GetNativeObject())->GetSpring();

		*__output = tmp__output;
	}

	void ScriptDistanceJoint::InternalSetSpring(ScriptDistanceJoint* self, Spring* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CDistanceJoint*>(self->GetNativeObject())->SetSpring(*value);
	}

	void ScriptDistanceJoint::InternalSetFlag(ScriptDistanceJoint* self, DistanceJointFlag flag, bool enabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CDistanceJoint*>(self->GetNativeObject())->SetFlag(flag, enabled);
	}

	bool ScriptDistanceJoint::InternalHasFlag(ScriptDistanceJoint* self, DistanceJointFlag flag)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CDistanceJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
