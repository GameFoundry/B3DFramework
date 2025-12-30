//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptD6Joint.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DD6Joint.h"
#include "B3DScriptD6JointDrive.generated.h"
#include "B3DScriptLimitLinear.generated.h"
#include "B3DScriptLimitAngularRange.generated.h"
#include "B3DScriptLimitConeRange.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTQuaternion.generated.h"

namespace b3d
{
	ScriptD6Joint::ScriptD6Joint(const TGameObjectHandle<D6Joint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptD6Joint::~ScriptD6Joint()
	{
		UnregisterEvents();
	}

	void ScriptD6Joint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMotion", (void*)&ScriptD6Joint::InternalSetMotion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMotion", (void*)&ScriptD6Joint::InternalGetMotion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTwist", (void*)&ScriptD6Joint::InternalGetTwist);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSwingY", (void*)&ScriptD6Joint::InternalGetSwingY);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSwingZ", (void*)&ScriptD6Joint::InternalGetSwingZ);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimitLinear", (void*)&ScriptD6Joint::InternalSetLimitLinear);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimitLinear", (void*)&ScriptD6Joint::InternalGetLimitLinear);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimitTwist", (void*)&ScriptD6Joint::InternalSetLimitTwist);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimitTwist", (void*)&ScriptD6Joint::InternalGetLimitTwist);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLimitSwing", (void*)&ScriptD6Joint::InternalSetLimitSwing);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLimitSwing", (void*)&ScriptD6Joint::InternalGetLimitSwing);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDrive", (void*)&ScriptD6Joint::InternalSetDrive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDrive", (void*)&ScriptD6Joint::InternalGetDrive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDrivePosition", (void*)&ScriptD6Joint::InternalGetDrivePosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDriveRotation", (void*)&ScriptD6Joint::InternalGetDriveRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDriveTransform", (void*)&ScriptD6Joint::InternalSetDriveTransform);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDriveLinearVelocity", (void*)&ScriptD6Joint::InternalGetDriveLinearVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDriveAngularVelocity", (void*)&ScriptD6Joint::InternalGetDriveAngularVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDriveVelocity", (void*)&ScriptD6Joint::InternalSetDriveVelocity);

	}

	MonoObject* ScriptD6Joint::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptD6Joint::InternalSetMotion(ScriptD6Joint* self, D6JointAxis axis, D6JointMotion motion)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<D6Joint*>(self->GetNativeObject())->SetMotion(axis, motion);
	}

	D6JointMotion ScriptD6Joint::InternalGetMotion(ScriptD6Joint* self, D6JointAxis axis)
	{
		D6JointMotion tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetMotion(axis);

		D6JointMotion __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptD6Joint::InternalGetTwist(ScriptD6Joint* self, TRadian<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TRadian<float> tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetTwist();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetSwingY(ScriptD6Joint* self, TRadian<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TRadian<float> tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetSwingY();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetSwingZ(ScriptD6Joint* self, TRadian<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TRadian<float> tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetSwingZ();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalSetLimitLinear(ScriptD6Joint* self, __LimitLinearInterop* limit)
	{
		if(!self->IsNativeObjectValid())
			return;

		LimitLinear tmplimit;
		tmplimit = ScriptLimitLinear::FromInterop(*limit);
		static_cast<D6Joint*>(self->GetNativeObject())->SetLimitLinear(tmplimit);
	}

	void ScriptD6Joint::InternalGetLimitLinear(ScriptD6Joint* self, __LimitLinearInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		LimitLinear tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetLimitLinear();

		__LimitLinearInterop interop__output;
		interop__output = ScriptLimitLinear::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitLinear::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptD6Joint::InternalSetLimitTwist(ScriptD6Joint* self, __LimitAngularRangeInterop* limit)
	{
		if(!self->IsNativeObjectValid())
			return;

		LimitAngularRange tmplimit;
		tmplimit = ScriptLimitAngularRange::FromInterop(*limit);
		static_cast<D6Joint*>(self->GetNativeObject())->SetLimitTwist(tmplimit);
	}

	void ScriptD6Joint::InternalGetLimitTwist(ScriptD6Joint* self, __LimitAngularRangeInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		LimitAngularRange tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetLimitTwist();

		__LimitAngularRangeInterop interop__output;
		interop__output = ScriptLimitAngularRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitAngularRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptD6Joint::InternalSetLimitSwing(ScriptD6Joint* self, __LimitConeRangeInterop* limit)
	{
		if(!self->IsNativeObjectValid())
			return;

		LimitConeRange tmplimit;
		tmplimit = ScriptLimitConeRange::FromInterop(*limit);
		static_cast<D6Joint*>(self->GetNativeObject())->SetLimitSwing(tmplimit);
	}

	void ScriptD6Joint::InternalGetLimitSwing(ScriptD6Joint* self, __LimitConeRangeInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		LimitConeRange tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetLimitSwing();

		__LimitConeRangeInterop interop__output;
		interop__output = ScriptLimitConeRange::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptLimitConeRange::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptD6Joint::InternalSetDrive(ScriptD6Joint* self, D6JointDriveType type, __D6JointDriveInterop* drive)
	{
		if(!self->IsNativeObjectValid())
			return;

		D6JointDrive tmpdrive;
		tmpdrive = ScriptD6JointDrive::FromInterop(*drive);
		static_cast<D6Joint*>(self->GetNativeObject())->SetDrive(type, tmpdrive);
	}

	void ScriptD6Joint::InternalGetDrive(ScriptD6Joint* self, D6JointDriveType type, __D6JointDriveInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		D6JointDrive tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetDrive(type);

		__D6JointDriveInterop interop__output;
		interop__output = ScriptD6JointDrive::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptD6JointDrive::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptD6Joint::InternalGetDrivePosition(ScriptD6Joint* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetDrivePosition();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetDriveRotation(ScriptD6Joint* self, TQuaternion<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TQuaternion<float> tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetDriveRotation();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalSetDriveTransform(ScriptD6Joint* self, TVector3<float>* position, TQuaternion<float>* rotation)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<D6Joint*>(self->GetNativeObject())->SetDriveTransform(*position, *rotation);
	}

	void ScriptD6Joint::InternalGetDriveLinearVelocity(ScriptD6Joint* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetDriveLinearVelocity();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalGetDriveAngularVelocity(ScriptD6Joint* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<D6Joint*>(self->GetNativeObject())->GetDriveAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptD6Joint::InternalSetDriveVelocity(ScriptD6Joint* self, TVector3<float>* linear, TVector3<float>* angular)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<D6Joint*>(self->GetNativeObject())->SetDriveVelocity(*linear, *angular);
	}
}
