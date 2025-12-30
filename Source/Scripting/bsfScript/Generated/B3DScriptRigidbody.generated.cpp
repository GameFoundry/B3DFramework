//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRigidbody.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DRigidbody.h"
#include "B3DScriptCollisionData.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTQuaternion.generated.h"

namespace b3d
{
	ScriptRigidbody::OnCollisionBeginThunkDefinition ScriptRigidbody::OnCollisionBeginThunk; 
	ScriptRigidbody::OnCollisionStayThunkDefinition ScriptRigidbody::OnCollisionStayThunk; 
	ScriptRigidbody::OnCollisionEndThunkDefinition ScriptRigidbody::OnCollisionEndThunk; 

	ScriptRigidbody::ScriptRigidbody(const TGameObjectHandle<Rigidbody>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRigidbody::~ScriptRigidbody()
	{
		UnregisterEvents();
	}

	void ScriptRigidbody::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptRigidbody::InternalMove);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Rotate", (void*)&ScriptRigidbody::InternalRotate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMass", (void*)&ScriptRigidbody::InternalSetMass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMass", (void*)&ScriptRigidbody::InternalGetMass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsKinematic", (void*)&ScriptRigidbody::InternalSetIsKinematic);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIsKinematic", (void*)&ScriptRigidbody::InternalGetIsKinematic);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsSleeping", (void*)&ScriptRigidbody::InternalIsSleeping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Sleep", (void*)&ScriptRigidbody::InternalSleep);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WakeUp", (void*)&ScriptRigidbody::InternalWakeUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSleepThreshold", (void*)&ScriptRigidbody::InternalSetSleepThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSleepThreshold", (void*)&ScriptRigidbody::InternalGetSleepThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUseGravity", (void*)&ScriptRigidbody::InternalSetUseGravity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUseGravity", (void*)&ScriptRigidbody::InternalGetUseGravity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVelocity", (void*)&ScriptRigidbody::InternalSetVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVelocity", (void*)&ScriptRigidbody::InternalGetVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAngularVelocity", (void*)&ScriptRigidbody::InternalSetAngularVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAngularVelocity", (void*)&ScriptRigidbody::InternalGetAngularVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDrag", (void*)&ScriptRigidbody::InternalSetDrag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDrag", (void*)&ScriptRigidbody::InternalGetDrag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAngularDrag", (void*)&ScriptRigidbody::InternalSetAngularDrag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAngularDrag", (void*)&ScriptRigidbody::InternalGetAngularDrag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInertiaTensor", (void*)&ScriptRigidbody::InternalSetInertiaTensor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInertiaTensor", (void*)&ScriptRigidbody::InternalGetInertiaTensor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxAngularVelocity", (void*)&ScriptRigidbody::InternalSetMaxAngularVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxAngularVelocity", (void*)&ScriptRigidbody::InternalGetMaxAngularVelocity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCenterOfMassPosition", (void*)&ScriptRigidbody::InternalSetCenterOfMassPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCenterOfMassPosition", (void*)&ScriptRigidbody::InternalGetCenterOfMassPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCenterOfMassRotation", (void*)&ScriptRigidbody::InternalSetCenterOfMassRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCenterOfMassRotation", (void*)&ScriptRigidbody::InternalGetCenterOfMassRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPositionSolverCount", (void*)&ScriptRigidbody::InternalSetPositionSolverCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPositionSolverCount", (void*)&ScriptRigidbody::InternalGetPositionSolverCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVelocitySolverCount", (void*)&ScriptRigidbody::InternalSetVelocitySolverCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVelocitySolverCount", (void*)&ScriptRigidbody::InternalGetVelocitySolverCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCollisionReportMode", (void*)&ScriptRigidbody::InternalSetCollisionReportMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCollisionReportMode", (void*)&ScriptRigidbody::InternalGetCollisionReportMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptRigidbody::InternalSetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptRigidbody::InternalGetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddForce", (void*)&ScriptRigidbody::InternalAddForce);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddTorque", (void*)&ScriptRigidbody::InternalAddTorque);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddForceAtPoint", (void*)&ScriptRigidbody::InternalAddForceAtPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVelocityAtPoint", (void*)&ScriptRigidbody::InternalGetVelocityAtPoint);

		OnCollisionBeginThunk = (OnCollisionBeginThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnCollisionBegin", "CollisionData&")->GetThunk();
		OnCollisionStayThunk = (OnCollisionStayThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnCollisionStay", "CollisionData&")->GetThunk();
		OnCollisionEndThunk = (OnCollisionEndThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnCollisionEnd", "CollisionData&")->GetThunk();
	}

	MonoObject* ScriptRigidbody::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptRigidbody::OnCollisionBegin(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionBeginThunk, GetScriptObject(), tmpp0);
	}

	void ScriptRigidbody::OnCollisionStay(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionStayThunk, GetScriptObject(), tmpp0);
	}

	void ScriptRigidbody::OnCollisionEnd(const CollisionData& p0)
	{
		MonoObject* tmpp0;
		__CollisionDataInterop interopp0;
		interopp0 = ScriptCollisionData::ToInterop(p0);
		tmpp0 = ScriptCollisionData::Box(interopp0);
		MonoUtil::InvokeThunk(OnCollisionEndThunk, GetScriptObject(), tmpp0);
	}

	void ScriptRigidbody::RegisterEvents()
	{
		OnCollisionBeginConnection = static_cast<Rigidbody*>(GetNativeObject())->OnCollisionBegin.Connect(std::bind(&ScriptRigidbody::OnCollisionBegin, this, std::placeholders::_1));
		OnCollisionStayConnection = static_cast<Rigidbody*>(GetNativeObject())->OnCollisionStay.Connect(std::bind(&ScriptRigidbody::OnCollisionStay, this, std::placeholders::_1));
		OnCollisionEndConnection = static_cast<Rigidbody*>(GetNativeObject())->OnCollisionEnd.Connect(std::bind(&ScriptRigidbody::OnCollisionEnd, this, std::placeholders::_1));
	}
	void ScriptRigidbody::UnregisterEvents()
	{
		OnCollisionBeginConnection.Disconnect();
		OnCollisionStayConnection.Disconnect();
		OnCollisionEndConnection.Disconnect();
	}
	void ScriptRigidbody::InternalMove(ScriptRigidbody* self, TVector3<float>* position)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->Move(*position);
	}

	void ScriptRigidbody::InternalRotate(ScriptRigidbody* self, TQuaternion<float>* rotation)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->Rotate(*rotation);
	}

	void ScriptRigidbody::InternalSetMass(ScriptRigidbody* self, float mass)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetMass(mass);
	}

	float ScriptRigidbody::InternalGetMass(ScriptRigidbody* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetIsKinematic(ScriptRigidbody* self, bool kinematic)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetIsKinematic(kinematic);
	}

	bool ScriptRigidbody::InternalGetIsKinematic(ScriptRigidbody* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetIsKinematic();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptRigidbody::InternalIsSleeping(ScriptRigidbody* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->IsSleeping();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSleep(ScriptRigidbody* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->Sleep();
	}

	void ScriptRigidbody::InternalWakeUp(ScriptRigidbody* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->WakeUp();
	}

	void ScriptRigidbody::InternalSetSleepThreshold(ScriptRigidbody* self, float threshold)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetSleepThreshold(threshold);
	}

	float ScriptRigidbody::InternalGetSleepThreshold(ScriptRigidbody* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetSleepThreshold();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetUseGravity(ScriptRigidbody* self, bool gravity)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetUseGravity(gravity);
	}

	bool ScriptRigidbody::InternalGetUseGravity(ScriptRigidbody* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetUseGravity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetVelocity(ScriptRigidbody* self, TVector3<float>* velocity)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetVelocity(*velocity);
	}

	void ScriptRigidbody::InternalGetVelocity(ScriptRigidbody* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetVelocity();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetAngularVelocity(ScriptRigidbody* self, TVector3<float>* velocity)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetAngularVelocity(*velocity);
	}

	void ScriptRigidbody::InternalGetAngularVelocity(ScriptRigidbody* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetAngularVelocity();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetDrag(ScriptRigidbody* self, float drag)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetDrag(drag);
	}

	float ScriptRigidbody::InternalGetDrag(ScriptRigidbody* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetAngularDrag(ScriptRigidbody* self, float drag)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetAngularDrag(drag);
	}

	float ScriptRigidbody::InternalGetAngularDrag(ScriptRigidbody* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetAngularDrag();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetInertiaTensor(ScriptRigidbody* self, TVector3<float>* tensor)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetInertiaTensor(*tensor);
	}

	void ScriptRigidbody::InternalGetInertiaTensor(ScriptRigidbody* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetInertiaTensor();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetMaxAngularVelocity(ScriptRigidbody* self, float velocity)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetMaxAngularVelocity(velocity);
	}

	float ScriptRigidbody::InternalGetMaxAngularVelocity(ScriptRigidbody* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetMaxAngularVelocity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetCenterOfMassPosition(ScriptRigidbody* self, TVector3<float>* position)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetCenterOfMassPosition(*position);
	}

	void ScriptRigidbody::InternalGetCenterOfMassPosition(ScriptRigidbody* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetCenterOfMassPosition();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetCenterOfMassRotation(ScriptRigidbody* self, TQuaternion<float>* rotation)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetCenterOfMassRotation(*rotation);
	}

	void ScriptRigidbody::InternalGetCenterOfMassRotation(ScriptRigidbody* self, TQuaternion<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TQuaternion<float> tmp__output;
		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetCenterOfMassRotation();

		*__output = tmp__output;
	}

	void ScriptRigidbody::InternalSetPositionSolverCount(ScriptRigidbody* self, uint32_t count)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetPositionSolverCount(count);
	}

	uint32_t ScriptRigidbody::InternalGetPositionSolverCount(ScriptRigidbody* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetPositionSolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetVelocitySolverCount(ScriptRigidbody* self, uint32_t count)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetVelocitySolverCount(count);
	}

	uint32_t ScriptRigidbody::InternalGetVelocitySolverCount(ScriptRigidbody* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetVelocitySolverCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetCollisionReportMode(ScriptRigidbody* self, CollisionReportMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptRigidbody::InternalGetCollisionReportMode(ScriptRigidbody* self)
	{
		CollisionReportMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalSetFlags(ScriptRigidbody* self, RigidbodyFlag flags)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->SetFlags(flags);
	}

	RigidbodyFlag ScriptRigidbody::InternalGetFlags(ScriptRigidbody* self)
	{
		Flags<RigidbodyFlag> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetFlags();

		RigidbodyFlag __output;
		__output = (RigidbodyFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptRigidbody::InternalAddForce(ScriptRigidbody* self, TVector3<float>* force, ForceMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->AddForce(*force, mode);
	}

	void ScriptRigidbody::InternalAddTorque(ScriptRigidbody* self, TVector3<float>* torque, ForceMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->AddTorque(*torque, mode);
	}

	void ScriptRigidbody::InternalAddForceAtPoint(ScriptRigidbody* self, TVector3<float>* force, TVector3<float>* position, PointForceMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Rigidbody*>(self->GetNativeObject())->AddForceAtPoint(*force, *position, mode);
	}

	void ScriptRigidbody::InternalGetVelocityAtPoint(ScriptRigidbody* self, TVector3<float>* point, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Rigidbody*>(self->GetNativeObject())->GetVelocityAtPoint(*point);

		*__output = tmp__output;
	}
}
