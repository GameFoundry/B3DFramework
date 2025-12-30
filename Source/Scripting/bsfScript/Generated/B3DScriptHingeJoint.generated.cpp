//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptHingeJoint.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DHingeJoint.h"
#include "B3DScriptLimitAngularRange.generated.h"
#include "B3DScriptHingeJointDrive.generated.h"

namespace b3d
{
	ScriptHingeJoint::ScriptHingeJoint(const TGameObjectHandle<HingeJoint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptHingeJoint::~ScriptHingeJoint()
	{
		UnregisterEvents();
	}

	void ScriptHingeJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAngle", (void*)&ScriptHingeJoint::InternalGetAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptHingeJoint::InternalGetSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptHingeJoint::InternalSetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptHingeJoint::InternalGetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDrive", (void*)&ScriptHingeJoint::InternalSetDrive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDrive", (void*)&ScriptHingeJoint::InternalGetDrive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptHingeJoint::InternalSetFlag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptHingeJoint::InternalHasFlag);

	}

	MonoObject* ScriptHingeJoint::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptHingeJoint::InternalGetAngle(ScriptHingeJoint* self, TRadian<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TRadian<float> tmp__output;
		tmp__output = static_cast<HingeJoint*>(self->GetNativeObject())->GetAngle();

		*__output = tmp__output;
	}

	float ScriptHingeJoint::InternalGetSpeed(ScriptHingeJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<HingeJoint*>(self->GetNativeObject())->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptHingeJoint::InternalSetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* limit)
	{
		if(!self->IsNativeObjectValid())
			return;

		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::FromInterop(*limit);
		static_cast<HingeJoint*>(self->GetNativeObject())->SetLimit(tmplimit);
	}

	void ScriptHingeJoint::InternalGetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		LimitAngularRange tmp__output;
		tmp__output = static_cast<HingeJoint*>(self->GetNativeObject())->GetLimit();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitAngularRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptHingeJoint::InternalSetDrive(ScriptHingeJoint* self, HingeJointDrive* drive)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<HingeJoint*>(self->GetNativeObject())->SetDrive(*drive);
	}

	void ScriptHingeJoint::InternalGetDrive(ScriptHingeJoint* self, HingeJointDrive* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		HingeJointDrive tmp__output;
		tmp__output = static_cast<HingeJoint*>(self->GetNativeObject())->GetDrive();

		*__output = tmp__output;
	}

	void ScriptHingeJoint::InternalSetFlag(ScriptHingeJoint* self, HingeJointFlag flag, bool enabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<HingeJoint*>(self->GetNativeObject())->SetFlag(flag, enabled);
	}

	bool ScriptHingeJoint::InternalHasFlag(ScriptHingeJoint* self, HingeJointFlag flag)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<HingeJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
