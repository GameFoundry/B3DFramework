//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCJoint.h"
#include "BsScriptTQuaternion.generated.h"
#include "../../../Foundation/bsfCore/Components/BsCRigidbody.h"
#include "BsScriptCRigidbody.generated.h"
#include "BsScriptTVector3.generated.h"

namespace bs
{
	ScriptJointWrapperBase::OnJointBreakThunkDefinition ScriptJointWrapperBase::OnJointBreakThunk; 

	void ScriptJointWrapperBase::OnJointBreak()
	{
		MonoUtil::InvokeThunk(OnJointBreakThunk, GetScriptObject());
	}

	void ScriptJointWrapperBase::RegisterEvents()
	{
		static_cast<CJoint*>(GetNativeObject())->OnJointBreak.Connect(std::bind(&ScriptJointWrapperBase::OnJointBreak, this));
		ScriptGameObjectWrapper::RegisterEvents();
	}
	ScriptJoint::ScriptJoint(const GameObjectHandle<CJoint>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptJoint::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBody", (void*)&ScriptJoint::InternalGetBody);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBody", (void*)&ScriptJoint::InternalSetBody);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptJoint::InternalGetPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptJoint::InternalGetRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTransform", (void*)&ScriptJoint::InternalSetTransform);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBreakForce", (void*)&ScriptJoint::InternalGetBreakForce);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBreakForce", (void*)&ScriptJoint::InternalSetBreakForce);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBreakTorque", (void*)&ScriptJoint::InternalGetBreakTorque);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBreakTorque", (void*)&ScriptJoint::InternalSetBreakTorque);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableCollision", (void*)&ScriptJoint::InternalGetEnableCollision);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableCollision", (void*)&ScriptJoint::InternalSetEnableCollision);

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
	MonoObject* ScriptJoint::InternalGetBody(ScriptJointWrapperBase* self, JointBody body)
	{
		GameObjectHandle<CRigidbody> tmp__output;
		tmp__output = static_cast<CJoint*>(self->GetNativeObject())->GetBody(body);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
			temp__output = ScriptComponent::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBody(ScriptJointWrapperBase* self, JointBody body, MonoObject* value)
	{
		GameObjectHandle<CRigidbody> tmpvalue;
		ScriptRigidbody* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRigidbody::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticGameObjectCast<CRigidbody>(scriptObjectWrappervalue->GetBaseNativeObjectAsHandle());
		static_cast<CJoint*>(self->GetNativeObject())->SetBody(body, tmpvalue);
	}

	void ScriptJoint::InternalGetPosition(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CJoint*>(self->GetNativeObject())->GetPosition(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalGetRotation(ScriptJointWrapperBase* self, JointBody body, TQuaternion<float>* __output)
	{
		TQuaternion<float> tmp__output;
		tmp__output = static_cast<CJoint*>(self->GetNativeObject())->GetRotation(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalSetTransform(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* position, TQuaternion<float>* rotation)
	{
		static_cast<CJoint*>(self->GetNativeObject())->SetTransform(body, *position, *rotation);
	}

	float ScriptJoint::InternalGetBreakForce(ScriptJointWrapperBase* self)
	{
		float tmp__output;
		tmp__output = static_cast<CJoint*>(self->GetNativeObject())->GetBreakForce();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBreakForce(ScriptJointWrapperBase* self, float force)
	{
		static_cast<CJoint*>(self->GetNativeObject())->SetBreakForce(force);
	}

	float ScriptJoint::InternalGetBreakTorque(ScriptJointWrapperBase* self)
	{
		float tmp__output;
		tmp__output = static_cast<CJoint*>(self->GetNativeObject())->GetBreakTorque();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBreakTorque(ScriptJointWrapperBase* self, float torque)
	{
		static_cast<CJoint*>(self->GetNativeObject())->SetBreakTorque(torque);
	}

	bool ScriptJoint::InternalGetEnableCollision(ScriptJointWrapperBase* self)
	{
		bool tmp__output;
		tmp__output = static_cast<CJoint*>(self->GetNativeObject())->GetEnableCollision();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetEnableCollision(ScriptJointWrapperBase* self, bool value)
	{
		static_cast<CJoint*>(self->GetNativeObject())->SetEnableCollision(value);
	}
}
