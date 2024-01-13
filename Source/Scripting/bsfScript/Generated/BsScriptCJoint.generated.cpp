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
	ScriptJointBase::OnJointBreakThunkDef ScriptJointBase::OnJointBreakThunk; 

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
		value->OnJointBreak.Connect(std::bind(&ScriptJoint::OnJointBreak, this));
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

		OnJointBreakThunk = (OnJointBreakThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnJointBreak", "")->GetThunk();
	}

	MonoObject* ScriptJoint::InternalGetBody(ScriptJointBase* thisPtr, JointBody body)
	{
		GameObjectHandle<CRigidbody> tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetBody(body);

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

	void ScriptJoint::InternalSetBody(ScriptJointBase* thisPtr, JointBody body, MonoObject* value)
	{
		GameObjectHandle<CRigidbody> tmpvalue;
		ScriptRigidbody* scriptvalue;
		scriptvalue = ScriptRigidbody::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetHandle();
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetBody(body, tmpvalue);
	}

	void ScriptJoint::InternalGetPosition(ScriptJointBase* thisPtr, JointBody body, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetPosition(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalGetRotation(ScriptJointBase* thisPtr, JointBody body, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetRotation(body);

		*__output = tmp__output;
	}

	void ScriptJoint::InternalSetTransform(ScriptJointBase* thisPtr, JointBody body, TVector3<float>* position, Quaternion* rotation)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetTransform(body, *position, *rotation);
	}

	float ScriptJoint::InternalGetBreakForce(ScriptJointBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetBreakForce();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBreakForce(ScriptJointBase* thisPtr, float force)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetBreakForce(force);
	}

	float ScriptJoint::InternalGetBreakTorque(ScriptJointBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetBreakTorque();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetBreakTorque(ScriptJointBase* thisPtr, float torque)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetBreakTorque(torque);
	}

	bool ScriptJoint::InternalGetEnableCollision(ScriptJointBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetEnableCollision();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptJoint::InternalSetEnableCollision(ScriptJointBase* thisPtr, bool value)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetEnableCollision(value);
	}
}
