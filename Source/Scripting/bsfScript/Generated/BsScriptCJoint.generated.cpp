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
	ScriptCJointBase::ScriptCJointBase(MonoObject* managedInstance)
		:ScriptComponentBase(managedInstance)
	 { }

	ScriptCJoint::OnJointBreakThunkDef ScriptCJoint::OnJointBreakThunk; 

	ScriptCJoint::ScriptCJoint(MonoObject* managedInstance, const GameObjectHandle<CJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->OnJointBreak.Connect(std::bind(&ScriptCJoint::OnJointBreak, this));
	}

	void ScriptCJoint::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetBody", (void*)&ScriptCJoint::InternalGetBody);
		metaData.ScriptClass->AddInternalCall("Internal_SetBody", (void*)&ScriptCJoint::InternalSetBody);
		metaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptCJoint::InternalGetPosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptCJoint::InternalGetRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetTransform", (void*)&ScriptCJoint::InternalSetTransform);
		metaData.ScriptClass->AddInternalCall("Internal_GetBreakForce", (void*)&ScriptCJoint::InternalGetBreakForce);
		metaData.ScriptClass->AddInternalCall("Internal_SetBreakForce", (void*)&ScriptCJoint::InternalSetBreakForce);
		metaData.ScriptClass->AddInternalCall("Internal_GetBreakTorque", (void*)&ScriptCJoint::InternalGetBreakTorque);
		metaData.ScriptClass->AddInternalCall("Internal_SetBreakTorque", (void*)&ScriptCJoint::InternalSetBreakTorque);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableCollision", (void*)&ScriptCJoint::InternalGetEnableCollision);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableCollision", (void*)&ScriptCJoint::InternalSetEnableCollision);

		OnJointBreakThunk = (OnJointBreakThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnJointBreak", "")->GetThunk();
	}

	void ScriptCJoint::OnJointBreak()
	{
		MonoUtil::InvokeThunk(OnJointBreakThunk, GetManagedInstance());
	}
	MonoObject* ScriptCJoint::InternalGetBody(ScriptCJointBase* thisPtr, JointBody body)
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

	void ScriptCJoint::InternalSetBody(ScriptCJointBase* thisPtr, JointBody body, MonoObject* value)
	{
		GameObjectHandle<CRigidbody> tmpvalue;
		ScriptCRigidbody* scriptvalue;
		scriptvalue = ScriptCRigidbody::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetHandle();
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetBody(body, tmpvalue);
	}

	void ScriptCJoint::InternalGetPosition(ScriptCJointBase* thisPtr, JointBody body, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetPosition(body);

		*__output = tmp__output;
	}

	void ScriptCJoint::InternalGetRotation(ScriptCJointBase* thisPtr, JointBody body, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetRotation(body);

		*__output = tmp__output;
	}

	void ScriptCJoint::InternalSetTransform(ScriptCJointBase* thisPtr, JointBody body, Vector3* position, Quaternion* rotation)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetTransform(body, *position, *rotation);
	}

	float ScriptCJoint::InternalGetBreakForce(ScriptCJointBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetBreakForce();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCJoint::InternalSetBreakForce(ScriptCJointBase* thisPtr, float force)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetBreakForce(force);
	}

	float ScriptCJoint::InternalGetBreakTorque(ScriptCJointBase* thisPtr)
	{
		float tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetBreakTorque();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCJoint::InternalSetBreakTorque(ScriptCJointBase* thisPtr, float torque)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetBreakTorque(torque);
	}

	bool ScriptCJoint::InternalGetEnableCollision(ScriptCJointBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->GetEnableCollision();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCJoint::InternalSetEnableCollision(ScriptCJointBase* thisPtr, bool value)
	{
		B3DStaticGameObjectCast<CJoint>(thisPtr->GetComponent())->SetEnableCollision(value);
	}
}
