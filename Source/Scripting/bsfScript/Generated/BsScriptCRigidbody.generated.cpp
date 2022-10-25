//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCRigidbody.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCRigidbody.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"
#include "BsScriptCollisionData.generated.h"

namespace bs
{
	ScriptCRigidbody::OnCollisionBeginThunkDef ScriptCRigidbody::OnCollisionBeginThunk;
	ScriptCRigidbody::OnCollisionStayThunkDef ScriptCRigidbody::OnCollisionStayThunk;
	ScriptCRigidbody::OnCollisionEndThunkDef ScriptCRigidbody::OnCollisionEndThunk;

	ScriptCRigidbody::ScriptCRigidbody(MonoObject* managedInstance, const GameObjectHandle<CRigidbody>& value)
		: TScriptComponent(managedInstance, value)
	{
		value->OnCollisionBegin.Connect(std::bind(&ScriptCRigidbody::OnCollisionBegin, this, std::placeholders::_1));
		value->OnCollisionStay.Connect(std::bind(&ScriptCRigidbody::OnCollisionStay, this, std::placeholders::_1));
		value->OnCollisionEnd.Connect(std::bind(&ScriptCRigidbody::OnCollisionEnd, this, std::placeholders::_1));
	}

	void ScriptCRigidbody::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptCRigidbody::InternalMove);
		metaData.ScriptClass->AddInternalCall("Internal_Rotate", (void*)&ScriptCRigidbody::InternalRotate);
		metaData.ScriptClass->AddInternalCall("Internal_SetMass", (void*)&ScriptCRigidbody::InternalSetMass);
		metaData.ScriptClass->AddInternalCall("Internal_GetMass", (void*)&ScriptCRigidbody::InternalGetMass);
		metaData.ScriptClass->AddInternalCall("Internal_SetIsKinematic", (void*)&ScriptCRigidbody::InternalSetIsKinematic);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsKinematic", (void*)&ScriptCRigidbody::InternalGetIsKinematic);
		metaData.ScriptClass->AddInternalCall("Internal_IsSleeping", (void*)&ScriptCRigidbody::InternalIsSleeping);
		metaData.ScriptClass->AddInternalCall("Internal_Sleep", (void*)&ScriptCRigidbody::InternalSleep);
		metaData.ScriptClass->AddInternalCall("Internal_WakeUp", (void*)&ScriptCRigidbody::InternalWakeUp);
		metaData.ScriptClass->AddInternalCall("Internal_SetSleepThreshold", (void*)&ScriptCRigidbody::InternalSetSleepThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_GetSleepThreshold", (void*)&ScriptCRigidbody::InternalGetSleepThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseGravity", (void*)&ScriptCRigidbody::InternalSetUseGravity);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseGravity", (void*)&ScriptCRigidbody::InternalGetUseGravity);
		metaData.ScriptClass->AddInternalCall("Internal_SetVelocity", (void*)&ScriptCRigidbody::InternalSetVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetVelocity", (void*)&ScriptCRigidbody::InternalGetVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetAngularVelocity", (void*)&ScriptCRigidbody::InternalSetAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetAngularVelocity", (void*)&ScriptCRigidbody::InternalGetAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetDrag", (void*)&ScriptCRigidbody::InternalSetDrag);
		metaData.ScriptClass->AddInternalCall("Internal_GetDrag", (void*)&ScriptCRigidbody::InternalGetDrag);
		metaData.ScriptClass->AddInternalCall("Internal_SetAngularDrag", (void*)&ScriptCRigidbody::InternalSetAngularDrag);
		metaData.ScriptClass->AddInternalCall("Internal_GetAngularDrag", (void*)&ScriptCRigidbody::InternalGetAngularDrag);
		metaData.ScriptClass->AddInternalCall("Internal_SetInertiaTensor", (void*)&ScriptCRigidbody::InternalSetInertiaTensor);
		metaData.ScriptClass->AddInternalCall("Internal_GetInertiaTensor", (void*)&ScriptCRigidbody::InternalGetInertiaTensor);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxAngularVelocity", (void*)&ScriptCRigidbody::InternalSetMaxAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxAngularVelocity", (void*)&ScriptCRigidbody::InternalGetMaxAngularVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenterOfMassPosition", (void*)&ScriptCRigidbody::InternalSetCenterOfMassPosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenterOfMassPosition", (void*)&ScriptCRigidbody::InternalGetCenterOfMassPosition);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenterOfMassRotation", (void*)&ScriptCRigidbody::InternalSetCenterOfMassRotation);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenterOfMassRotation", (void*)&ScriptCRigidbody::InternalGetCenterOfMassRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetPositionSolverCount", (void*)&ScriptCRigidbody::InternalSetPositionSolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetPositionSolverCount", (void*)&ScriptCRigidbody::InternalGetPositionSolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetVelocitySolverCount", (void*)&ScriptCRigidbody::InternalSetVelocitySolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetVelocitySolverCount", (void*)&ScriptCRigidbody::InternalGetVelocitySolverCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetCollisionReportMode", (void*)&ScriptCRigidbody::InternalSetCollisionReportMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetCollisionReportMode", (void*)&ScriptCRigidbody::InternalGetCollisionReportMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptCRigidbody::InternalSetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptCRigidbody::InternalGetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_AddForce", (void*)&ScriptCRigidbody::InternalAddForce);
		metaData.ScriptClass->AddInternalCall("Internal_AddTorque", (void*)&ScriptCRigidbody::InternalAddTorque);
		metaData.ScriptClass->AddInternalCall("Internal_AddForceAtPoint", (void*)&ScriptCRigidbody::InternalAddForceAtPoint);
		metaData.ScriptClass->AddInternalCall("Internal_GetVelocityAtPoint", (void*)&ScriptCRigidbody::InternalGetVelocityAtPoint);

		OnCollisionBeginThunk = (OnCollisionBeginThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
	}

	void ScriptCRigidbody::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionBeginThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCRigidbody::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionStayThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCRigidbody::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionEndThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptCRigidbody::InternalMove(ScriptCRigidbody* thisPtr, Vector3* position)
	{
		thisPtr->GetHandle()->Move(*position);
	}

	void ScriptCRigidbody::InternalRotate(ScriptCRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->GetHandle()->Rotate(*rotation);
	}

	void ScriptCRigidbody::InternalSetMass(ScriptCRigidbody* thisPtr, float mass)
	{
		thisPtr->GetHandle()->SetMass(mass);
	}

	float ScriptCRigidbody::InternalGetMass(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetIsKinematic(ScriptCRigidbody* thisPtr, bool kinematic)
	{
		thisPtr->GetHandle()->SetIsKinematic(kinematic);
	}

	bool ScriptCRigidbody::InternalGetIsKinematic(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetIsKinematic();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCRigidbody::InternalIsSleeping(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->IsSleeping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSleep(ScriptCRigidbody* thisPtr)
	{
		thisPtr->GetHandle()->Sleep();
	}

	void ScriptCRigidbody::InternalWakeUp(ScriptCRigidbody* thisPtr)
	{
		thisPtr->GetHandle()->WakeUp();
	}

	void ScriptCRigidbody::InternalSetSleepThreshold(ScriptCRigidbody* thisPtr, float threshold)
	{
		thisPtr->GetHandle()->SetSleepThreshold(threshold);
	}

	float ScriptCRigidbody::InternalGetSleepThreshold(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSleepThreshold();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetUseGravity(ScriptCRigidbody* thisPtr, bool gravity)
	{
		thisPtr->GetHandle()->SetUseGravity(gravity);
	}

	bool ScriptCRigidbody::InternalGetUseGravity(ScriptCRigidbody* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetUseGravity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->GetHandle()->SetVelocity(*velocity);
	}

	void ScriptCRigidbody::InternalGetVelocity(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVelocity();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::InternalSetAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity)
	{
		thisPtr->GetHandle()->SetAngularVelocity(*velocity);
	}

	void ScriptCRigidbody::InternalGetAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::InternalSetDrag(ScriptCRigidbody* thisPtr, float drag)
	{
		thisPtr->GetHandle()->SetDrag(drag);
	}

	float ScriptCRigidbody::InternalGetDrag(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetAngularDrag(ScriptCRigidbody* thisPtr, float drag)
	{
		thisPtr->GetHandle()->SetAngularDrag(drag);
	}

	float ScriptCRigidbody::InternalGetAngularDrag(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAngularDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* tensor)
	{
		thisPtr->GetHandle()->SetInertiaTensor(*tensor);
	}

	void ScriptCRigidbody::InternalGetInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetInertiaTensor();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::InternalSetMaxAngularVelocity(ScriptCRigidbody* thisPtr, float maxVelocity)
	{
		thisPtr->GetHandle()->SetMaxAngularVelocity(maxVelocity);
	}

	float ScriptCRigidbody::InternalGetMaxAngularVelocity(ScriptCRigidbody* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMaxAngularVelocity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* position)
	{
		thisPtr->GetHandle()->SetCenterOfMassPosition(*position);
	}

	void ScriptCRigidbody::InternalGetCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenterOfMassPosition();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::InternalSetCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* rotation)
	{
		thisPtr->GetHandle()->SetCenterOfMassRotation(*rotation);
	}

	void ScriptCRigidbody::InternalGetCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCenterOfMassRotation();

		*__output = tmp__output;
	}

	void ScriptCRigidbody::InternalSetPositionSolverCount(ScriptCRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->SetPositionSolverCount(count);
	}

	uint32_t ScriptCRigidbody::InternalGetPositionSolverCount(ScriptCRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPositionSolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetVelocitySolverCount(ScriptCRigidbody* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->SetVelocitySolverCount(count);
	}

	uint32_t ScriptCRigidbody::InternalGetVelocitySolverCount(ScriptCRigidbody* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVelocitySolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetCollisionReportMode(ScriptCRigidbody* thisPtr, CollisionReportMode mode)
	{
		thisPtr->GetHandle()->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptCRigidbody::InternalGetCollisionReportMode(ScriptCRigidbody* thisPtr)
	{
		CollisionReportMode tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalSetFlags(ScriptCRigidbody* thisPtr, RigidbodyFlag flags)
	{
		thisPtr->GetHandle()->SetFlags(flags);
	}

	RigidbodyFlag ScriptCRigidbody::InternalGetFlags(ScriptCRigidbody* thisPtr)
	{
		RigidbodyFlag tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFlags();

		RigidbodyFlag __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRigidbody::InternalAddForce(ScriptCRigidbody* thisPtr, Vector3* force, ForceMode mode)
	{
		thisPtr->GetHandle()->AddForce(*force, mode);
	}

	void ScriptCRigidbody::InternalAddTorque(ScriptCRigidbody* thisPtr, Vector3* torque, ForceMode mode)
	{
		thisPtr->GetHandle()->AddTorque(*torque, mode);
	}

	void ScriptCRigidbody::InternalAddForceAtPoint(ScriptCRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode)
	{
		thisPtr->GetHandle()->AddForceAtPoint(*force, *position, mode);
	}

	void ScriptCRigidbody::InternalGetVelocityAtPoint(ScriptCRigidbody* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVelocityAtPoint(*point);

		*__output = tmp__output;
	}
} // namespace bs
