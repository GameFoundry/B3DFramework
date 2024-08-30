//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCHingeJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCHingeJoint.h"
#include "BsScriptLimitAngularRange.generated.h"
#include "BsScriptHingeJointDrive.generated.h"

namespace bs
{
	ScriptHingeJoint::ScriptHingeJoint(const GameObjectHandle<CHingeJoint>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptHingeJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAngle", (void*)&ScriptHingeJoint::InternalGetAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptHingeJoint::InternalGetSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimit", (void*)&ScriptHingeJoint::InternalGetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimit", (void*)&ScriptHingeJoint::InternalSetLimit);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDrive", (void*)&ScriptHingeJoint::InternalGetDrive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDrive", (void*)&ScriptHingeJoint::InternalSetDrive);
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
		TRadian<float> tmp__output;
		tmp__output = static_cast<CHingeJoint*>(self->GetNativeObject())->GetAngle();

		*__output = tmp__output;
	}

	float ScriptHingeJoint::InternalGetSpeed(ScriptHingeJoint* self)
	{
		float tmp__output;
		tmp__output = static_cast<CHingeJoint*>(self->GetNativeObject())->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptHingeJoint::InternalGetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* __output)
	{
		LimitAngularRange tmp__output;
		tmp__output = static_cast<CHingeJoint*>(self->GetNativeObject())->GetLimit();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitAngularRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptHingeJoint::InternalSetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* limit)
	{
		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::FromInterop(*limit);
		static_cast<CHingeJoint*>(self->GetNativeObject())->SetLimit(tmplimit);
	}

	void ScriptHingeJoint::InternalGetDrive(ScriptHingeJoint* self, HingeJointDrive* __output)
	{
		HingeJointDrive tmp__output;
		tmp__output = static_cast<CHingeJoint*>(self->GetNativeObject())->GetDrive();

		*__output = tmp__output;
	}

	void ScriptHingeJoint::InternalSetDrive(ScriptHingeJoint* self, HingeJointDrive* drive)
	{
		static_cast<CHingeJoint*>(self->GetNativeObject())->SetDrive(*drive);
	}

	void ScriptHingeJoint::InternalSetFlag(ScriptHingeJoint* self, HingeJointFlag flag, bool enabled)
	{
		static_cast<CHingeJoint*>(self->GetNativeObject())->SetFlag(flag, enabled);
	}

	bool ScriptHingeJoint::InternalHasFlag(ScriptHingeJoint* self, HingeJointFlag flag)
	{
		bool tmp__output;
		tmp__output = static_cast<CHingeJoint*>(self->GetNativeObject())->HasFlag(flag);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
