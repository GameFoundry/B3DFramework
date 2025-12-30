//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSliderJoint.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DSliderJoint.h"
#include "B3DScriptLimitLinearRange.generated.h"

namespace b3d
{
	ScriptSliderJoint::ScriptSliderJoint(const TGameObjectHandle<SliderJoint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSliderJoint::~ScriptSliderJoint()
	{
		UnregisterEvents();
	}

	void ScriptSliderJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptSliderJoint::InternalGetPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptSliderJoint::InternalGetSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptSliderJoint::InternalSetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptSliderJoint::InternalGetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptSliderJoint::InternalSetFlag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptSliderJoint::InternalHasFlag);

	}

	MonoObject* ScriptSliderJoint::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	float ScriptSliderJoint::InternalGetPosition(ScriptSliderJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SliderJoint*>(self->GetNativeObject())->GetPosition();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptSliderJoint::InternalGetSpeed(ScriptSliderJoint* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SliderJoint*>(self->GetNativeObject())->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSliderJoint::InternalSetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* limit)
	{
		if(!self->IsNativeObjectValid())
			return;

		LimitLinearRange tmplimit;
		tmplimit = ScriptLimitLinearRange::FromInterop(*limit);
		static_cast<SliderJoint*>(self->GetNativeObject())->SetLimit(tmplimit);
	}

	void ScriptSliderJoint::InternalGetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		LimitLinearRange tmp__output;
		tmp__output = static_cast<SliderJoint*>(self->GetNativeObject())->GetLimit();

		__LimitLinearRangeInterop interop__output;
		interop__output = ScriptLimitLinearRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitLinearRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptSliderJoint::InternalSetFlag(ScriptSliderJoint* self, SliderJointFlag flag, bool enabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SliderJoint*>(self->GetNativeObject())->SetFlag(flag, enabled);
	}

	bool ScriptSliderJoint::InternalHasFlag(ScriptSliderJoint* self, SliderJointFlag flag)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SliderJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
