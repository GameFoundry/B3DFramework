//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCRigidbody.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCRigidbody.h"
#include "BsScriptCollisionData.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptRigidbody::OnCollisionBeginThunkDef ScriptRigidbody::OnCollisionBeginThunk; 
	ScriptRigidbody::OnCollisionStayThunkDef ScriptRigidbody::OnCollisionStayThunk; 
	ScriptRigidbody::OnCollisionEndThunkDef ScriptRigidbody::OnCollisionEndThunk; 

	ScriptRigidbody::ScriptRigidbody(MonoObject* managedInstance, const GameObjectHandle<CRigidbody>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->OnCollisionBegin.Connect(std::bind(&ScriptRigidbody::OnCollisionBegin, this, std::placeholders::_1));
		value->OnCollisionStay.Connect(std::bind(&ScriptRigidbody::OnCollisionStay, this, std::placeholders::_1));
		value->OnCollisionEnd.Connect(std::bind(&ScriptRigidbody::OnCollisionEnd, this, std::placeholders::_1));
	}

	void ScriptRigidbody::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptRigidbody::InternalMove);
		metaData.ScriptClass->AddInternalCall("Internal_Rotate", (void*)&ScriptRigidbody::InternalRotate);
		metaData.ScriptClass->AddInternalCall("Internal_SetMass", (void*)&ScriptRigidbody::InternalSetMass);
		metaData.ScriptClass->AddInternalCall("Internal_GetMass", (void*)&ScriptRigidbody::InternalGetMass);
		metaData.ScriptClass->AddInternalCall("Internal_SetIsKinematic", (void*)&ScriptRigidbody::InternalSetIsKinematic);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsKinematic", (void*)&ScriptRigidbody::InternalGetIsKinematic);
		metaData.ScriptClass->AddInternalCall("Internal_IsSleeping", (void*)&ScriptRigidbody::InternalIsSleeping);
		metaData.ScriptClass->AddInternalCall("Internal_Sleep", (void*)&ScriptRigidbody::InternalSleep);
		metaData.ScriptClass->AddInternalCall("Internal_WakeUp", (void*)&ScriptRigidbody::InternalWakeUp);
		metaData.ScriptClass->AddInternalCall("Internal_SetSleepThreshold", (void*)&ScriptRigidbody::InternalSetSleepThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_GetSleepThreshold", (void*)&ScriptRigidbody::InternalGetSleepThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseGravity", (void*)&ScriptRigidbody::InternalSetUseGravity);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseGravity", (void*)&ScriptRigidbody::InternalGetUseGravity);
		metaData.ScriptClass->AddInternalCall("Internal_SetVelocity", (void*)&ScriptRigidbody::InternalSetVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetVelocity", (void*)&ScriptRigidbody::InternalGetVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetAngularVelocity", (void*)&ScriptRigidbody::InternalSetAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetAngularVelocity", (void*)&ScriptRigidbody::InternalGetAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetDrag", (void*)&ScriptRigidbody::InternalSetDrag);
		metaData.ScriptClass->AddInternalCall("Internal_GetDrag", (void*)&ScriptRigidbody::InternalGetDrag);
		metaData.ScriptClass->AddInternalCall("Internal_SetAngularDrag", (void*)&ScriptRigidbody::InternalSetAngularDrag);
		metaData.ScriptClass->AddInternalCall("Internal_GetAngularDrag", (void*)&ScriptRigidbody::InternalGetAngularDrag);
		metaData.ScriptClass->AddInternalCall("Internal_SetInertiaTensor", (void*)&ScriptRigidbody::InternalSetInertiaTensor);
		metaData.ScriptClass->AddInternalCall("Internal_GetInertiaTensor", (void*)&ScriptRigidbody::InternalGetInertiaTensor);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxAngularVelocity", (void*)&ScriptRigidbody::InternalSetMaxAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxAngularVelocity", (void*)&ScriptRigidbody::InternalGetMaxAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenterOfMassPosition", (void*)&ScriptRigidbody::InternalSetCenterOfMassPosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenterOfMassPosition", (void*)&ScriptRigidbody::InternalGetCenterOfMassPosition);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenterOfMassRotation", (void*)&ScriptRigidbody::InternalSetCenterOfMassRotation);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenterOfMassRotation", (void*)&ScriptRigidbody::InternalGetCenterOfMassRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetPositionSolverCount", (void*)&ScriptRigidbody::InternalSetPositionSolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetPositionSolverCount", (void*)&ScriptRigidbody::InternalGetPositionSolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetVelocitySolverCount", (void*)&ScriptRigidbody::InternalSetVelocitySolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetVelocitySolverCount", (void*)&ScriptRigidbody::InternalGetVelocitySolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetCollisionReportMode", (void*)&ScriptRigidbody::InternalSetCollisionReportMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetCollisionReportMode", (void*)&ScriptRigidbody::InternalGetCollisionReportMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptRigidbody::InternalSetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptRigidbody::InternalGetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_AddForce", (void*)&ScriptRigidbody::InternalAddForce);
		metaData.ScriptClass->AddInternalCall("Internal_AddTorque", (void*)&ScriptRigidbody::InternalAddTorque);
		metaData.ScriptClass->AddInternalCall("Internal_AddForceAtPoint", (void*)&ScriptRigidbody::InternalAddForceAtPoint);
		metaData.ScriptClass->AddInternalCall("Internal_GetVelocityAtPoint", (void*)&ScriptRigidbody::InternalGetVelocityAtPoint);

		OnCollisionBeginThunk = (OnCollisionBeginThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
	}

	void ScriptRigidbody::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionBeginThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptRigidbody::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionStayThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptRigidbody::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionEndThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptRigidbody::InternalMove(ScriptRigidbody* thisPtr, TVector3<float>* position)
	{
		thisPtr->GetHandle()->Move(*position);
	}

	void ScriptRigidbody::InternalRotate(ScriptRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->GetHandle()->Rotate(*rotation);
	}

	void ScriptRigidbody::InternalSetMass(ScriptRigidbody* thisPtr, float mass)
	{
		thisPtr->GetHandle()->SetMass(mass);
	}

	float ScriptRigidbody::InternalGetMass(ScriptRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetIsKinematic(ScriptRigidbody* thisPtr, bool kinematic)
	{
		thisPtr->GetHandle()->SetIsKinematic(kinematic);
	}

	bool ScriptRigidbody::InternalGetIsKinematic(ScriptRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetIsKinematic();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptRigidbody::InternalIsSleeping(ScriptRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->IsSleeping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSleep(ScriptRigidbody* thisPtr)
	{
		thisPtr->GetHandle()->Sleep();
	}

	void ScriptRigidbody::InternalWakeUp(ScriptRigidbody* thisPtr)
	{
		thisPtr->GetHandle()->WakeUp();
	}

	void ScriptRigidbody::InternalSetSleepThreshold(ScriptRigidbody* thisPtr, float threshold)
	{
		thisPtr->GetHandle()->SetSleepThreshold(threshold);
	}

	float ScriptRigidbody::InternalGetSleepThreshold(ScriptRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSleepThreshold();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetUseGravity(ScriptRigidbody* thisPtr, bool gravity)
	{
		thisPtr->GetHandle()->SetUseGravity(gravity);
	}

	bool ScriptRigidbody::InternalGetUseGravity(ScriptRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetUseGravity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetVelocity(ScriptRigidbody* thisPtr, TVector3<float>* velocity)
	{
		thisPtr->GetHandle()->SetVelocity(*velocity);
	}

	void ScriptRigidbody::InternalGetVelocity(ScriptRigidbody* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVelocity();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetAngularVelocity(ScriptRigidbody* thisPtr, TVector3<float>* velocity)
	{
		thisPtr->GetHandle()->SetAngularVelocity(*velocity);
	}

	void ScriptRigidbody::InternalGetAngularVelocity(ScriptRigidbody* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetDrag(ScriptRigidbody* thisPtr, float drag)
	{
		thisPtr->GetHandle()->SetDrag(drag);
	}

	float ScriptRigidbody::InternalGetDrag(ScriptRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetAngularDrag(ScriptRigidbody* thisPtr, float drag)
	{
		thisPtr->GetHandle()->SetAngularDrag(drag);
	}

	float ScriptRigidbody::InternalGetAngularDrag(ScriptRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAngularDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetInertiaTensor(ScriptRigidbody* thisPtr, TVector3<float>* tensor)
	{
		thisPtr->GetHandle()->SetInertiaTensor(*tensor);
	}

	void ScriptRigidbody::InternalGetInertiaTensor(ScriptRigidbody* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetInertiaTensor();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetMaxAngularVelocity(ScriptRigidbody* thisPtr, float maxVelocity)
	{
		thisPtr->GetHandle()->SetMaxAngularVelocity(maxVelocity);
	}

	float ScriptRigidbody::InternalGetMaxAngularVelocity(ScriptRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMaxAngularVelocity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetCenterOfMassPosition(ScriptRigidbody* thisPtr, TVector3<float>* position)
	{
		thisPtr->GetHandle()->SetCenterOfMassPosition(*position);
	}

	void ScriptRigidbody::InternalGetCenterOfMassPosition(ScriptRigidbody* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenterOfMassPosition();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetCenterOfMassRotation(ScriptRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->GetHandle()->SetCenterOfMassRotation(*rotation);
	}

	void ScriptRigidbody::InternalGetCenterOfMassRotation(ScriptRigidbody* thisPtr, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenterOfMassRotation();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetPositionSolverCount(ScriptRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->SetPositionSolverCount(count);
	}

	uint32_t ScriptRigidbody::InternalGetPositionSolverCount(ScriptRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPositionSolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetVelocitySolverCount(ScriptRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->SetVelocitySolverCount(count);
	}

	uint32_t ScriptRigidbody::InternalGetVelocitySolverCount(ScriptRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVelocitySolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetCollisionReportMode(ScriptRigidbody* thisPtr, CollisionReportMode mode)
	{
		thisPtr->GetHandle()->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptRigidbody::InternalGetCollisionReportMode(ScriptRigidbody* thisPtr)
	{
		CollisionReportMode tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetFlags(ScriptRigidbody* thisPtr, RigidbodyFlag flags)
	{
		thisPtr->GetHandle()->SetFlags(flags);
	}

	RigidbodyFlag ScriptRigidbody::InternalGetFlags(ScriptRigidbody* thisPtr)
	{
		RigidbodyFlag tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFlags();

		RigidbodyFlag __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalAddForce(ScriptRigidbody* thisPtr, TVector3<float>* force, ForceMode mode)
	{
		thisPtr->GetHandle()->AddForce(*force, mode);
	}

	void ScriptRigidbody::InternalAddTorque(ScriptRigidbody* thisPtr, TVector3<float>* torque, ForceMode mode)
	{
		thisPtr->GetHandle()->AddTorque(*torque, mode);
	}

	void ScriptRigidbody::InternalAddForceAtPoint(ScriptRigidbody* thisPtr, TVector3<float>* force, TVector3<float>* position, PointForceMode mode)
	{
		thisPtr->GetHandle()->AddForceAtPoint(*force, *position, mode);
	}

	void ScriptRigidbody::InternalGetVelocityAtPoint(ScriptRigidbody* thisPtr, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVelocityAtPoint(*point);

		*__output = tmp__output;
	}
}
