//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	ScriptCJoint::onJointBreakThunkDef ScriptCJoint::onJointBreakThunk; 

	ScriptCJoint::ScriptCJoint(MonoObject* managedInstance, const GameObjectHandle<CJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->onJointBreak.Connect(std::bind(&ScriptCJoint::onJointBreak, this));
	}

	void ScriptCJoint::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getBody", (void*)&ScriptCJoint::Internal_getBody);
		metaData.scriptClass->AddInternalCall("Internal_setBody", (void*)&ScriptCJoint::Internal_setBody);
		metaData.scriptClass->AddInternalCall("Internal_getPosition", (void*)&ScriptCJoint::Internal_getPosition);
		metaData.scriptClass->AddInternalCall("Internal_getRotation", (void*)&ScriptCJoint::Internal_getRotation);
		metaData.scriptClass->AddInternalCall("Internal_setTransform", (void*)&ScriptCJoint::Internal_setTransform);
		metaData.scriptClass->AddInternalCall("Internal_getBreakForce", (void*)&ScriptCJoint::Internal_getBreakForce);
		metaData.scriptClass->AddInternalCall("Internal_setBreakForce", (void*)&ScriptCJoint::Internal_setBreakForce);
		metaData.scriptClass->AddInternalCall("Internal_getBreakTorque", (void*)&ScriptCJoint::Internal_getBreakTorque);
		metaData.scriptClass->AddInternalCall("Internal_setBreakTorque", (void*)&ScriptCJoint::Internal_setBreakTorque);
		metaData.scriptClass->AddInternalCall("Internal_getEnableCollision", (void*)&ScriptCJoint::Internal_getEnableCollision);
		metaData.scriptClass->AddInternalCall("Internal_setEnableCollision", (void*)&ScriptCJoint::Internal_setEnableCollision);

		onJointBreakThunk = (onJointBreakThunkDef)metaData.scriptClass->GetMethodExact("Internal_onJointBreak", "")->getThunk();
	}

	void ScriptCJoint::OnJointBreak()
	{
		MonoUtil::invokeThunk(onJointBreakThunk, getManagedInstance());
	}
	MonoObject* ScriptCJoint::Internal_getBody(ScriptCJointBase* thisPtr, JointBody body)
	{
		GameObjectHandle<CRigidbody> tmp__output;
		tmp__output = static_object_cast<CJoint>(thisPtr->GetComponent())->getBody(body);

		MonoObject* __output;
		ScriptComponentBase* script__output = nullptr;
		if(tmp__output)
			script__output = ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(tmp__output));
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCJoint::Internal_setBody(ScriptCJointBase* thisPtr, JointBody body, MonoObject* value)
	{
		GameObjectHandle<CRigidbody> tmpvalue;
		ScriptCRigidbody* scriptvalue;
		scriptvalue = ScriptCRigidbody::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetHandle();
		static_object_cast<CJoint>(thisPtr->GetComponent())->setBody(body, tmpvalue);
	}

	void ScriptCJoint::Internal_getPosition(ScriptCJointBase* thisPtr, JointBody body, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = static_object_cast<CJoint>(thisPtr->GetComponent())->getPosition(body);

		*__output = tmp__output;
	}

	void ScriptCJoint::Internal_getRotation(ScriptCJointBase* thisPtr, JointBody body, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = static_object_cast<CJoint>(thisPtr->GetComponent())->getRotation(body);

		*__output = tmp__output;
	}

	void ScriptCJoint::Internal_setTransform(ScriptCJointBase* thisPtr, JointBody body, Vector3* position, Quaternion* rotation)
	{
		static_object_cast<CJoint>(thisPtr->GetComponent())->setTransform(body, *position, *rotation);
	}

	float ScriptCJoint::Internal_getBreakForce(ScriptCJointBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_object_cast<CJoint>(thisPtr->GetComponent())->getBreakForce();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCJoint::Internal_setBreakForce(ScriptCJointBase* thisPtr, float force)
	{
		static_object_cast<CJoint>(thisPtr->GetComponent())->setBreakForce(force);
	}

	float ScriptCJoint::Internal_getBreakTorque(ScriptCJointBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_object_cast<CJoint>(thisPtr->GetComponent())->getBreakTorque();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCJoint::Internal_setBreakTorque(ScriptCJointBase* thisPtr, float torque)
	{
		static_object_cast<CJoint>(thisPtr->GetComponent())->setBreakTorque(torque);
	}

	bool ScriptCJoint::Internal_getEnableCollision(ScriptCJointBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = static_object_cast<CJoint>(thisPtr->GetComponent())->getEnableCollision();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCJoint::Internal_setEnableCollision(ScriptCJointBase* thisPtr, bool value)
	{
		static_object_cast<CJoint>(thisPtr->GetComponent())->setEnableCollision(value);
	}
}
