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
	ScriptRigidbody::OnCollisionBeginThunkDefinition ScriptRigidbody::OnCollisionBeginThunk; 
	ScriptRigidbody::OnCollisionStayThunkDefinition ScriptRigidbody::OnCollisionStayThunk; 
	ScriptRigidbody::OnCollisionEndThunkDefinition ScriptRigidbody::OnCollisionEndThunk; 

	ScriptRigidbody::ScriptRigidbody(MonoObject* managedInstance, const GameObjectHandle<CRigidbody>& value)
		:TScriptComponent(managedInstance, value)
	{
		static_cast<GameObjectHandle<CRigidbody>>(value)->OnCollisionBegin.Connect(std::bind(&ScriptRigidbody::OnCollisionBegin, this, std::placeholders::_1));
		static_cast<GameObjectHandle<CRigidbody>>(value)->OnCollisionStay.Connect(std::bind(&ScriptRigidbody::OnCollisionStay, this, std::placeholders::_1));
		static_cast<GameObjectHandle<CRigidbody>>(value)->OnCollisionEnd.Connect(std::bind(&ScriptRigidbody::OnCollisionEnd, this, std::placeholders::_1));
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

		OnCollisionBeginThunk = (OnCollisionBeginThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
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

	void ScriptRigidbody::InternalMove(ScriptRigidbody* self, TVector3<float>* position)
	{
		self->GetHandle()->Move(*position);
	}

	void ScriptRigidbody::InternalRotate(ScriptRigidbody* self, Quaternion* rotation)
	{
		self->GetHandle()->Rotate(*rotation);
	}

	void ScriptRigidbody::InternalSetMass(ScriptRigidbody* self, float mass)
	{
		self->GetHandle()->SetMass(mass);
	}

	float ScriptRigidbody::InternalGetMass(ScriptRigidbody* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetIsKinematic(ScriptRigidbody* self, bool kinematic)
	{
		self->GetHandle()->SetIsKinematic(kinematic);
	}

	bool ScriptRigidbody::InternalGetIsKinematic(ScriptRigidbody* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->GetIsKinematic();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptRigidbody::InternalIsSleeping(ScriptRigidbody* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->IsSleeping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSleep(ScriptRigidbody* self)
	{
		self->GetHandle()->Sleep();
	}

	void ScriptRigidbody::InternalWakeUp(ScriptRigidbody* self)
	{
		self->GetHandle()->WakeUp();
	}

	void ScriptRigidbody::InternalSetSleepThreshold(ScriptRigidbody* self, float threshold)
	{
		self->GetHandle()->SetSleepThreshold(threshold);
	}

	float ScriptRigidbody::InternalGetSleepThreshold(ScriptRigidbody* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetSleepThreshold();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetUseGravity(ScriptRigidbody* self, bool gravity)
	{
		self->GetHandle()->SetUseGravity(gravity);
	}

	bool ScriptRigidbody::InternalGetUseGravity(ScriptRigidbody* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->GetUseGravity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetVelocity(ScriptRigidbody* self, TVector3<float>* velocity)
	{
		self->GetHandle()->SetVelocity(*velocity);
	}

	void ScriptRigidbody::InternalGetVelocity(ScriptRigidbody* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetVelocity();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetAngularVelocity(ScriptRigidbody* self, TVector3<float>* velocity)
	{
		self->GetHandle()->SetAngularVelocity(*velocity);
	}

	void ScriptRigidbody::InternalGetAngularVelocity(ScriptRigidbody* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetDrag(ScriptRigidbody* self, float drag)
	{
		self->GetHandle()->SetDrag(drag);
	}

	float ScriptRigidbody::InternalGetDrag(ScriptRigidbody* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetAngularDrag(ScriptRigidbody* self, float drag)
	{
		self->GetHandle()->SetAngularDrag(drag);
	}

	float ScriptRigidbody::InternalGetAngularDrag(ScriptRigidbody* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetAngularDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetInertiaTensor(ScriptRigidbody* self, TVector3<float>* tensor)
	{
		self->GetHandle()->SetInertiaTensor(*tensor);
	}

	void ScriptRigidbody::InternalGetInertiaTensor(ScriptRigidbody* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetInertiaTensor();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetMaxAngularVelocity(ScriptRigidbody* self, float maxVelocity)
	{
		self->GetHandle()->SetMaxAngularVelocity(maxVelocity);
	}

	float ScriptRigidbody::InternalGetMaxAngularVelocity(ScriptRigidbody* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetMaxAngularVelocity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetCenterOfMassPosition(ScriptRigidbody* self, TVector3<float>* position)
	{
		self->GetHandle()->SetCenterOfMassPosition(*position);
	}

	void ScriptRigidbody::InternalGetCenterOfMassPosition(ScriptRigidbody* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetCenterOfMassPosition();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetCenterOfMassRotation(ScriptRigidbody* self, Quaternion* rotation)
	{
		self->GetHandle()->SetCenterOfMassRotation(*rotation);
	}

	void ScriptRigidbody::InternalGetCenterOfMassRotation(ScriptRigidbody* self, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = self->GetHandle()->GetCenterOfMassRotation();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetPositionSolverCount(ScriptRigidbody* self, uint32_t count)
	{
		self->GetHandle()->SetPositionSolverCount(count);
	}

	uint32_t ScriptRigidbody::InternalGetPositionSolverCount(ScriptRigidbody* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetPositionSolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetVelocitySolverCount(ScriptRigidbody* self, uint32_t count)
	{
		self->GetHandle()->SetVelocitySolverCount(count);
	}

	uint32_t ScriptRigidbody::InternalGetVelocitySolverCount(ScriptRigidbody* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetVelocitySolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetCollisionReportMode(ScriptRigidbody* self, CollisionReportMode mode)
	{
		self->GetHandle()->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptRigidbody::InternalGetCollisionReportMode(ScriptRigidbody* self)
	{
		CollisionReportMode tmp__output;
		tmp__output = self->GetHandle()->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetFlags(ScriptRigidbody* self, RigidbodyFlag flags)
	{
		self->GetHandle()->SetFlags(flags);
	}

	RigidbodyFlag ScriptRigidbody::InternalGetFlags(ScriptRigidbody* self)
	{
		RigidbodyFlag tmp__output;
		tmp__output = self->GetHandle()->GetFlags();

		RigidbodyFlag __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalAddForce(ScriptRigidbody* self, TVector3<float>* force, ForceMode mode)
	{
		self->GetHandle()->AddForce(*force, mode);
	}

	void ScriptRigidbody::InternalAddTorque(ScriptRigidbody* self, TVector3<float>* torque, ForceMode mode)
	{
		self->GetHandle()->AddTorque(*torque, mode);
	}

	void ScriptRigidbody::InternalAddForceAtPoint(ScriptRigidbody* self, TVector3<float>* force, TVector3<float>* position, PointForceMode mode)
	{
		self->GetHandle()->AddForceAtPoint(*force, *position, mode);
	}

	void ScriptRigidbody::InternalGetVelocityAtPoint(ScriptRigidbody* self, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetVelocityAtPoint(*point);

		*__output = tmp__output;
	}
}
