//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSliderJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCSliderJoint.h"
#include "BsScriptLimitLinearRange.generated.h"

namespace bs
{
	ScriptSliderJoint::ScriptSliderJoint(const GameObjectHandle<CSliderJoint>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptSliderJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptSliderJoint::InternalGetPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptSliderJoint::InternalGetSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptSliderJoint::InternalGetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptSliderJoint::InternalSetLimit);
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
		tmp__output = static_cast<CSliderJoint*>(self->GetNativeObject())->GetPosition();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptSliderJoint::InternalGetSpeed(ScriptSliderJoint* self)
	{
		float tmp__output;
		tmp__output = static_cast<CSliderJoint*>(self->GetNativeObject())->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSliderJoint::InternalGetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* __output)
	{
		LimitLinearRange tmp__output;
		tmp__output = static_cast<CSliderJoint*>(self->GetNativeObject())->GetLimit();

		__LimitLinearRangeInterop interop__output;
		interop__output = ScriptLimitLinearRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitLinearRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptSliderJoint::InternalSetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* limit)
	{
		LimitLinearRange tmplimit;
		tmplimit = ScriptLimitLinearRange::FromInterop(*limit);
		static_cast<CSliderJoint*>(self->GetNativeObject())->SetLimit(tmplimit);
	}

	void ScriptSliderJoint::InternalSetFlag(ScriptSliderJoint* self, SliderJointFlag flag, bool enabled)
	{
		static_cast<CSliderJoint*>(self->GetNativeObject())->SetFlag(flag, enabled);
	}

	bool ScriptSliderJoint::InternalHasFlag(ScriptSliderJoint* self, SliderJointFlag flag)
	{
		bool tmp__output;
		tmp__output = static_cast<CSliderJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
