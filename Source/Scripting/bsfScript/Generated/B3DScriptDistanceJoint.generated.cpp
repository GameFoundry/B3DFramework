//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptDistanceJoint.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DDistanceJoint.h"
#include "B3DScriptSpring.generated.h"

namespace b3d
{
	ScriptDistanceJoint::ScriptDistanceJoint(const TGameObjectHandle<DistanceJoint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptDistanceJoint::~ScriptDistanceJoint()
	{
		UnregisterEvents();
	}

	void ScriptDistanceJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDistance", (void*)&ScriptDistanceJoint::InternalGetDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMinDistance", (void*)&ScriptDistanceJoint::InternalSetMinDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinDistance", (void*)&ScriptDistanceJoint::InternalGetMinDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxDistance", (void*)&ScriptDistanceJoint::InternalSetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxDistance", (void*)&ScriptDistanceJoint::InternalGetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTolerance", (void*)&ScriptDistanceJoint::InternalSetTolerance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTolerance", (void*)&ScriptDistanceJoint::InternalGetTolerance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpring", (void*)&ScriptDistanceJoint::InternalSetSpring);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpring", (void*)&ScriptDistanceJoint::InternalGetSpring);
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

		tmp__output = static_cast<DistanceJoint*>(self->GetNativeObject())->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetMinDistance(ScriptDistanceJoint* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DistanceJoint*>(self->GetNativeObject())->SetMinDistance(value);
	}

	float ScriptDistanceJoint::InternalGetMinDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DistanceJoint*>(self->GetNativeObject())->GetMinDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetMaxDistance(ScriptDistanceJoint* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DistanceJoint*>(self->GetNativeObject())->SetMaxDistance(value);
	}

	float ScriptDistanceJoint::InternalGetMaxDistance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DistanceJoint*>(self->GetNativeObject())->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetTolerance(ScriptDistanceJoint* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DistanceJoint*>(self->GetNativeObject())->SetTolerance(value);
	}

	float ScriptDistanceJoint::InternalGetTolerance(ScriptDistanceJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DistanceJoint*>(self->GetNativeObject())->GetTolerance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDistanceJoint::InternalSetSpring(ScriptDistanceJoint* self, Spring* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DistanceJoint*>(self->GetNativeObject())->SetSpring(*value);
	}

	void ScriptDistanceJoint::InternalGetSpring(ScriptDistanceJoint* self, Spring* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		Spring tmp__output;
		tmp__output = static_cast<DistanceJoint*>(self->GetNativeObject())->GetSpring();

		*__output = tmp__output;
	}

	void ScriptDistanceJoint::InternalSetFlag(ScriptDistanceJoint* self, DistanceJointFlag flag, bool enabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DistanceJoint*>(self->GetNativeObject())->SetFlag(flag, enabled);
	}

	bool ScriptDistanceJoint::InternalHasFlag(ScriptDistanceJoint* self, DistanceJointFlag flag)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DistanceJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
