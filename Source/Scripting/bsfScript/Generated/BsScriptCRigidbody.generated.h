//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Physics/BsRigidbody.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "../../../Foundation/bsfCore/Physics/BsRigidbody.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "../../../Foundation/bsfCore/Physics/BsRigidbody.h"

namespace bs
{
	class CRigidbody;
}

namespace bs
{
	struct __CollisionDataInterop;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCRigidbody : public TScriptComponent<ScriptCRigidbody, CRigidbody>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Rigidbody")

		ScriptCRigidbody(MonoObject* managedInstance, const GameObjectHandle<CRigidbody>& value);

	private:
		void OnCollisionBegin(const CollisionData& p0);
		void OnCollisionStay(const CollisionData& p0);
		void OnCollisionEnd(const CollisionData& p0);

		typedef void(BS_THUNKCALL* OnCollisionBeginThunkDef)(MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionBeginThunkDef OnCollisionBeginThunk;
		typedef void(BS_THUNKCALL* OnCollisionStayThunkDef)(MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionStayThunkDef OnCollisionStayThunk;
		typedef void(BS_THUNKCALL* OnCollisionEndThunkDef)(MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionEndThunkDef OnCollisionEndThunk;

		static void InternalMove(ScriptCRigidbody* thisPtr, Vector3* position);
		static void InternalRotate(ScriptCRigidbody* thisPtr, Quaternion* rotation);
		static void InternalSetMass(ScriptCRigidbody* thisPtr, float mass);
		static float InternalGetMass(ScriptCRigidbody* thisPtr);
		static void InternalSetIsKinematic(ScriptCRigidbody* thisPtr, bool kinematic);
		static bool InternalGetIsKinematic(ScriptCRigidbody* thisPtr);
		static bool InternalIsSleeping(ScriptCRigidbody* thisPtr);
		static void InternalSleep(ScriptCRigidbody* thisPtr);
		static void InternalWakeUp(ScriptCRigidbody* thisPtr);
		static void InternalSetSleepThreshold(ScriptCRigidbody* thisPtr, float threshold);
		static float InternalGetSleepThreshold(ScriptCRigidbody* thisPtr);
		static void InternalSetUseGravity(ScriptCRigidbody* thisPtr, bool gravity);
		static bool InternalGetUseGravity(ScriptCRigidbody* thisPtr);
		static void InternalSetVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity);
		static void InternalGetVelocity(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void InternalSetAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity);
		static void InternalGetAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void InternalSetDrag(ScriptCRigidbody* thisPtr, float drag);
		static float InternalGetDrag(ScriptCRigidbody* thisPtr);
		static void InternalSetAngularDrag(ScriptCRigidbody* thisPtr, float drag);
		static float InternalGetAngularDrag(ScriptCRigidbody* thisPtr);
		static void InternalSetInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* tensor);
		static void InternalGetInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void InternalSetMaxAngularVelocity(ScriptCRigidbody* thisPtr, float maxVelocity);
		static float InternalGetMaxAngularVelocity(ScriptCRigidbody* thisPtr);
		static void InternalSetCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* position);
		static void InternalGetCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void InternalSetCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* rotation);
		static void InternalGetCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* __output);
		static void InternalSetPositionSolverCount(ScriptCRigidbody* thisPtr, uint32_t count);
		static uint32_t InternalGetPositionSolverCount(ScriptCRigidbody* thisPtr);
		static void InternalSetVelocitySolverCount(ScriptCRigidbody* thisPtr, uint32_t count);
		static uint32_t InternalGetVelocitySolverCount(ScriptCRigidbody* thisPtr);
		static void InternalSetCollisionReportMode(ScriptCRigidbody* thisPtr, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptCRigidbody* thisPtr);
		static void InternalSetFlags(ScriptCRigidbody* thisPtr, RigidbodyFlag flags);
		static RigidbodyFlag InternalGetFlags(ScriptCRigidbody* thisPtr);
		static void InternalAddForce(ScriptCRigidbody* thisPtr, Vector3* force, ForceMode mode);
		static void InternalAddTorque(ScriptCRigidbody* thisPtr, Vector3* torque, ForceMode mode);
		static void InternalAddForceAtPoint(ScriptCRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode);
		static void InternalGetVelocityAtPoint(ScriptCRigidbody* thisPtr, Vector3* point, Vector3* __output);
	};
} // namespace bs
