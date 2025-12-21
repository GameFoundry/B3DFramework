//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSphericalJoint.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DSphericalJoint.h"
#include "B3DScriptLimitConeRange.generated.h"

namespace b3d
{
	ScriptSphericalJoint::ScriptSphericalJoint(const GameObjectHandle<SphericalJoint>& nativeObject)
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
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptSphericalJoint::InternalSetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptSphericalJoint::InternalGetLimit);
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
	void ScriptSphericalJoint::InternalSetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* limit)
	{
		if(!self->IsNativeObjectValid())
			return;

		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::FromInterop(*limit);
		static_cast<SphericalJoint*>(self->GetNativeObject())->SetLimit(tmplimit);
	}

	void ScriptSphericalJoint::InternalGetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		LimitConeRange tmp__output;
		tmp__output = static_cast<SphericalJoint*>(self->GetNativeObject())->GetLimit();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitConeRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptSphericalJoint::InternalSetFlag(ScriptSphericalJoint* self, SphericalJointFlag flag, bool isEnabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SphericalJoint*>(self->GetNativeObject())->SetFlag(flag, isEnabled);
	}

	bool ScriptSphericalJoint::InternalHasFlag(ScriptSphericalJoint* self, SphericalJointFlag flag)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SphericalJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
