//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptJoint.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Components/B3DJoint.h"
#include "B3DScriptTVector3.generated.h"
#include "../../../Engine/Core/Components/B3DRigidbody.h"
#include "B3DScriptRigidbody.generated.h"
#include "B3DScriptTQuaternion.generated.h"

namespace b3d
{
	ScriptJointWrapperBase::OnJointBreakThunkDefinition ScriptJointWrapperBase::OnJointBreakThunk; 

	void ScriptJointWrapperBase::OnJointBreak()
	{
		MonoUtil::InvokeThunk(OnJointBreakThunk, GetScriptObject());
	}

	void ScriptJointWrapperBase::RegisterEvents()
	{
		OnJointBreakConnection = static_cast<Joint*>(GetNativeObject())->OnJointBreak.Connect([this]() { OnJointBreak(); });
		ScriptGameObjectWrapper::RegisterEvents();
	}
	void ScriptJointWrapperBase::UnregisterEvents()
	{
		OnJointBreakConnection.Disconnect();
		ScriptGameObjectWrapper::UnregisterEvents();
	}
	ScriptJoint::ScriptJoint(const TGameObjectHandle<Joint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptJoint::~ScriptJoint()
	{
		UnregisterEvents();
	}

	void ScriptJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBody", (void*)&ScriptJoint::InternalSetBody);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBody", (void*)&ScriptJoint::InternalGetBody);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRelativeBodyPosition", (void*)&ScriptJoint::InternalGetRelativeBodyPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRelativeBodyRotation", (void*)&ScriptJoint::InternalGetRelativeBodyRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRelativeBodyTransform", (void*)&ScriptJoint::InternalSetRelativeBodyTransform);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBreakForce", (void*)&ScriptJoint::InternalSetBreakForce);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBreakForce", (void*)&ScriptJoint::InternalGetBreakForce);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBreakTorque", (void*)&ScriptJoint::InternalSetBreakTorque);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBreakTorque", (void*)&ScriptJoint::InternalGetBreakTorque);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableCollision", (void*)&ScriptJoint::InternalSetEnableCollision);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableCollision", (void*)&ScriptJoint::InternalGetEnableCollision);

		OnJointBreakThunk = (OnJointBreakThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnJointBreak", "")->GetThunk();
	}

	MonoObject* ScriptJoint::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptJoint::InternalSetBody(ScriptJointWrapperBase* self, JointBody body, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TGameObjectHandle<Rigidbody> tmpvalue;
		ScriptRigidbody* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRigidbody::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticGameObjectCast<Rigidbody>(scriptObjectWrappervalue->GetBaseNativeObjectAsHandle());
		static_cast<Joint*>(self->GetNativeObject())->SetBody(body, tmpvalue);
	}

	MonoObject* ScriptJoint::InternalGetBody(ScriptJointWrapperBase* self, JointBody body)
	{
		TGameObjectHandle<Rigidbody> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Joint*>(self->GetNativeObject())->GetBody(body);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
			temp__output = ScriptComponent::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	void ScriptJoint::InternalGetRelativeBodyPosition(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Joint*>(self->GetNativeObject())->GetRelativeBodyPosition(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalGetRelativeBodyRotation(ScriptJointWrapperBase* self, JointBody body, TQuaternion<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TQuaternion<float> tmp__output;
		tmp__output = static_cast<Joint*>(self->GetNativeObject())->GetRelativeBodyRotation(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalSetRelativeBodyTransform(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* position, TQuaternion<float>* rotation)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Joint*>(self->GetNativeObject())->SetRelativeBodyTransform(body, *position, *rotation);
	}

	void ScriptJoint::InternalSetBreakForce(ScriptJointWrapperBase* self, float force)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Joint*>(self->GetNativeObject())->SetBreakForce(force);
	}

	float ScriptJoint::InternalGetBreakForce(ScriptJointWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Joint*>(self->GetNativeObject())->GetBreakForce();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBreakTorque(ScriptJointWrapperBase* self, float torque)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Joint*>(self->GetNativeObject())->SetBreakTorque(torque);
	}

	float ScriptJoint::InternalGetBreakTorque(ScriptJointWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Joint*>(self->GetNativeObject())->GetBreakTorque();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetEnableCollision(ScriptJointWrapperBase* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Joint*>(self->GetNativeObject())->SetEnableCollision(value);
	}

	bool ScriptJoint::InternalGetEnableCollision(ScriptJointWrapperBase* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Joint*>(self->GetNativeObject())->GetEnableCollision();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
