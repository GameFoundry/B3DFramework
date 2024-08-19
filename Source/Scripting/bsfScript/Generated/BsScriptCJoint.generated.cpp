//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCJoint.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptCRigidbody.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptJointBase::OnJointBreakThunkDefinition ScriptJointBase::OnJointBreakThunk; 

	ScriptJointBase::ScriptJointBase(MonoObject* managedInstance)
		:ScriptComponentBase(managedInstance)
	 { }

	void ScriptJointBase::OnJointBreak()
	{
		MonoUtil::InvokeThunk(OnJointBreakThunk, GetManagedInstance());
	}

	ScriptJoint::ScriptJoint(MonoObject* managedInstance, const GameObjectHandle<CJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
		static_cast<GameObjectHandle<CJoint>>(value)->OnJointBreak.Connect(std::bind(&ScriptJoint::OnJointBreak, this));
	}

	void ScriptJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetBody", (void*)&ScriptJoint::InternalGetBody);
		metaData.ScriptClass->AddInternalCall("Internal_SetBody", (void*)&ScriptJoint::InternalSetBody);
		metaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptJoint::InternalGetPosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptJoint::InternalGetRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetTransform", (void*)&ScriptJoint::InternalSetTransform);
		metaData.ScriptClass->AddInternalCall("Internal_GetBreakForce", (void*)&ScriptJoint::InternalGetBreakForce);
		metaData.ScriptClass->AddInternalCall("Internal_SetBreakForce", (void*)&ScriptJoint::InternalSetBreakForce);
		metaData.ScriptClass->AddInternalCall("Internal_GetBreakTorque", (void*)&ScriptJoint::InternalGetBreakTorque);
		metaData.ScriptClass->AddInternalCall("Internal_SetBreakTorque", (void*)&ScriptJoint::InternalSetBreakTorque);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableCollision", (void*)&ScriptJoint::InternalGetEnableCollision);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableCollision", (void*)&ScriptJoint::InternalSetEnableCollision);

		OnJointBreakThunk = (OnJointBreakThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnJointBreak", "")->GetThunk();
	}

	MonoObject* ScriptJoint::InternalGetBody(ScriptJointBase* self, JointBody body)
	{
		GameObjectHandle<CRigidbody> tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(self->GetComponent())->GetBody(body);

		MonoObject* __output;
		ScriptComponentBase* script__output = nullptr;
		if(tmp__output)
			script__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(B3DStaticGameObjectCast<Component>(tmp__output));
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptJoint::InternalSetBody(ScriptJointBase* self, JointBody body, MonoObject* value)
	{
		GameObjectHandle<CRigidbody> tmpvalue;
		ScriptRigidbody* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRigidbody::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetHandle();
		B3DStaticGameObjectCast<CJoint>(self->GetComponent())->SetBody(body, tmpvalue);
	}

	void ScriptJoint::InternalGetPosition(ScriptJointBase* self, JointBody body, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(self->GetComponent())->GetPosition(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalGetRotation(ScriptJointBase* self, JointBody body, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(self->GetComponent())->GetRotation(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalSetTransform(ScriptJointBase* self, JointBody body, TVector3<float>* position, Quaternion* rotation)
	{
		B3DStaticGameObjectCast<CJoint>(self->GetComponent())->SetTransform(body, *position, *rotation);
	}

	float ScriptJoint::InternalGetBreakForce(ScriptJointBase* self)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(self->GetComponent())->GetBreakForce();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBreakForce(ScriptJointBase* self, float force)
	{
		B3DStaticGameObjectCast<CJoint>(self->GetComponent())->SetBreakForce(force);
	}

	float ScriptJoint::InternalGetBreakTorque(ScriptJointBase* self)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(self->GetComponent())->GetBreakTorque();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBreakTorque(ScriptJointBase* self, float torque)
	{
		B3DStaticGameObjectCast<CJoint>(self->GetComponent())->SetBreakTorque(torque);
	}

	bool ScriptJoint::InternalGetEnableCollision(ScriptJointBase* self)
	{
		bool tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(self->GetComponent())->GetEnableCollision();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetEnableCollision(ScriptJointBase* self, bool value)
	{
		B3DStaticGameObjectCast<CJoint>(self->GetComponent())->SetEnableCollision(value);
	}
}
