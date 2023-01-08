//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

namespace bs { class CRigidbody; }
namespace bs { struct __CollisionDataInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRigidbody : public TScriptComponent<ScriptRigidbody, CRigidbody>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Rigidbody")

		ScriptRigidbody(MonoObject* managedInstance, const GameObjectHandle<CRigidbody>& value);

	private:
		void OnCollisionBegin(const CollisionData& p0);
		void OnCollisionStay(const CollisionData& p0);
		void OnCollisionEnd(const CollisionData& p0);

		typedef void(B3D_THUNKCALL *OnCollisionBeginThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionBeginThunkDef OnCollisionBeginThunk;
		typedef void(B3D_THUNKCALL *OnCollisionStayThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionStayThunkDef OnCollisionStayThunk;
		typedef void(B3D_THUNKCALL *OnCollisionEndThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionEndThunkDef OnCollisionEndThunk;

		static void InternalMove(ScriptRigidbody* thisPtr, Vector3* position);
		static void InternalRotate(ScriptRigidbody* thisPtr, Quaternion* rotation);
		static void InternalSetMass(ScriptRigidbody* thisPtr, float mass);
		static float InternalGetMass(ScriptRigidbody* thisPtr);
		static void InternalSetIsKinematic(ScriptRigidbody* thisPtr, bool kinematic);
		static bool InternalGetIsKinematic(ScriptRigidbody* thisPtr);
		static bool InternalIsSleeping(ScriptRigidbody* thisPtr);
		static void InternalSleep(ScriptRigidbody* thisPtr);
		static void InternalWakeUp(ScriptRigidbody* thisPtr);
		static void InternalSetSleepThreshold(ScriptRigidbody* thisPtr, float threshold);
		static float InternalGetSleepThreshold(ScriptRigidbody* thisPtr);
		static void InternalSetUseGravity(ScriptRigidbody* thisPtr, bool gravity);
		static bool InternalGetUseGravity(ScriptRigidbody* thisPtr);
		static void InternalSetVelocity(ScriptRigidbody* thisPtr, Vector3* velocity);
		static void InternalGetVelocity(ScriptRigidbody* thisPtr, Vector3* __output);
		static void InternalSetAngularVelocity(ScriptRigidbody* thisPtr, Vector3* velocity);
		static void InternalGetAngularVelocity(ScriptRigidbody* thisPtr, Vector3* __output);
		static void InternalSetDrag(ScriptRigidbody* thisPtr, float drag);
		static float InternalGetDrag(ScriptRigidbody* thisPtr);
		static void InternalSetAngularDrag(ScriptRigidbody* thisPtr, float drag);
		static float InternalGetAngularDrag(ScriptRigidbody* thisPtr);
		static void InternalSetInertiaTensor(ScriptRigidbody* thisPtr, Vector3* tensor);
		static void InternalGetInertiaTensor(ScriptRigidbody* thisPtr, Vector3* __output);
		static void InternalSetMaxAngularVelocity(ScriptRigidbody* thisPtr, float maxVelocity);
		static float InternalGetMaxAngularVelocity(ScriptRigidbody* thisPtr);
		static void InternalSetCenterOfMassPosition(ScriptRigidbody* thisPtr, Vector3* position);
		static void InternalGetCenterOfMassPosition(ScriptRigidbody* thisPtr, Vector3* __output);
		static void InternalSetCenterOfMassRotation(ScriptRigidbody* thisPtr, Quaternion* rotation);
		static void InternalGetCenterOfMassRotation(ScriptRigidbody* thisPtr, Quaternion* __output);
		static void InternalSetPositionSolverCount(ScriptRigidbody* thisPtr, uint32_t count);
		static uint32_t InternalGetPositionSolverCount(ScriptRigidbody* thisPtr);
		static void InternalSetVelocitySolverCount(ScriptRigidbody* thisPtr, uint32_t count);
		static uint32_t InternalGetVelocitySolverCount(ScriptRigidbody* thisPtr);
		static void InternalSetCollisionReportMode(ScriptRigidbody* thisPtr, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptRigidbody* thisPtr);
		static void InternalSetFlags(ScriptRigidbody* thisPtr, RigidbodyFlag flags);
		static RigidbodyFlag InternalGetFlags(ScriptRigidbody* thisPtr);
		static void InternalAddForce(ScriptRigidbody* thisPtr, Vector3* force, ForceMode mode);
		static void InternalAddTorque(ScriptRigidbody* thisPtr, Vector3* torque, ForceMode mode);
		static void InternalAddForceAtPoint(ScriptRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode);
		static void InternalGetVelocityAtPoint(ScriptRigidbody* thisPtr, Vector3* point, Vector3* __output);
	};
}
