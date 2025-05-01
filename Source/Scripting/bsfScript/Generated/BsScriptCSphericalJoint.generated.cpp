//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSphericalJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCSphericalJoint.h"
#include "BsScriptLimitConeRange.generated.h"

namespace bs
{
	ScriptSphericalJoint::ScriptSphericalJoint(const GameObjectHandle<CSphericalJoint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSphericalJoint::~ScriptSphericalJoint()
	{
		UnregisterEvents();
	}

	void ScriptSphericalJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptSphericalJoint::InternalGetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptSphericalJoint::InternalSetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlag", (void*)&ScriptSphericalJoint::InternalSetFlag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptSphericalJoint::InternalHasFlag);

	}

	MonoObject* ScriptSphericalJoint::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptSphericalJoint::InternalGetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		LimitConeRange tmp__output;
		tmp__output = static_cast<CSphericalJoint*>(self->GetNativeObject())->GetLimit();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitConeRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptSphericalJoint::InternalSetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* limit)
	{
		if(!self->IsNativeObjectValid())
			return;

		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::FromInterop(*limit);
		static_cast<CSphericalJoint*>(self->GetNativeObject())->SetLimit(tmplimit);
	}

	void ScriptSphericalJoint::InternalSetFlag(ScriptSphericalJoint* self, SphericalJointFlag flag, bool enabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CSphericalJoint*>(self->GetNativeObject())->SetFlag(flag, enabled);
	}

	bool ScriptSphericalJoint::InternalHasFlag(ScriptSphericalJoint* self, SphericalJointFlag flag)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CSphericalJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
