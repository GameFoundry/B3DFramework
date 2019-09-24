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
#include "../../../Foundation/bsfCore/Physics/BsRigidbody.h"

namespace bs { class CRigidbody; }
namespace bs { struct __CollisionDataInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCRigidbody : public TScriptComponent<ScriptCRigidbody, CRigidbody>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Rigidbody")

		ScriptCRigidbody(MonoObject* managedInstance, const GameObjectHandle<CRigidbody>& value);

	private:
		void onCollisionBegin(const CollisionData& p0);
		void onCollisionStay(const CollisionData& p0);
		void onCollisionEnd(const CollisionData& p0);

		typedef void(BS_THUNKCALL *onCollisionBeginThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static onCollisionBeginThunkDef onCollisionBeginThunk;
		typedef void(BS_THUNKCALL *onCollisionStayThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static onCollisionStayThunkDef onCollisionStayThunk;
		typedef void(BS_THUNKCALL *onCollisionEndThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static onCollisionEndThunkDef onCollisionEndThunk;

		static void Internal_move(ScriptCRigidbody* thisPtr, Vector3* position);
		static void Internal_rotate(ScriptCRigidbody* thisPtr, Quaternion* rotation);
		static void Internal_setMass(ScriptCRigidbody* thisPtr, float mass);
		static float Internal_getMass(ScriptCRigidbody* thisPtr);
		static void Internal_setIsKinematic(ScriptCRigidbody* thisPtr, bool kinematic);
		static bool Internal_getIsKinematic(ScriptCRigidbody* thisPtr);
		static bool Internal_isSleeping(ScriptCRigidbody* thisPtr);
		static void Internal_sleep(ScriptCRigidbody* thisPtr);
		static void Internal_wakeUp(ScriptCRigidbody* thisPtr);
		static void Internal_setSleepThreshold(ScriptCRigidbody* thisPtr, float threshold);
		static float Internal_getSleepThreshold(ScriptCRigidbody* thisPtr);
		static void Internal_setUseGravity(ScriptCRigidbody* thisPtr, bool gravity);
		static bool Internal_getUseGravity(ScriptCRigidbody* thisPtr);
		static void Internal_setVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity);
		static void Internal_getVelocity(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void Internal_setAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* velocity);
		static void Internal_getAngularVelocity(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void Internal_setDrag(ScriptCRigidbody* thisPtr, float drag);
		static float Internal_getDrag(ScriptCRigidbody* thisPtr);
		static void Internal_setAngularDrag(ScriptCRigidbody* thisPtr, float drag);
		static float Internal_getAngularDrag(ScriptCRigidbody* thisPtr);
		static void Internal_setInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* tensor);
		static void Internal_getInertiaTensor(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void Internal_setMaxAngularVelocity(ScriptCRigidbody* thisPtr, float maxVelocity);
		static float Internal_getMaxAngularVelocity(ScriptCRigidbody* thisPtr);
		static void Internal_setCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* position);
		static void Internal_getCenterOfMassPosition(ScriptCRigidbody* thisPtr, Vector3* __output);
		static void Internal_setCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* rotation);
		static void Internal_getCenterOfMassRotation(ScriptCRigidbody* thisPtr, Quaternion* __output);
		static void Internal_setPositionSolverCount(ScriptCRigidbody* thisPtr, uint32_t count);
		static uint32_t Internal_getPositionSolverCount(ScriptCRigidbody* thisPtr);
		static void Internal_setVelocitySolverCount(ScriptCRigidbody* thisPtr, uint32_t count);
		static uint32_t Internal_getVelocitySolverCount(ScriptCRigidbody* thisPtr);
		static void Internal_setCollisionReportMode(ScriptCRigidbody* thisPtr, CollisionReportMode mode);
		static CollisionReportMode Internal_getCollisionReportMode(ScriptCRigidbody* thisPtr);
		static void Internal_setFlags(ScriptCRigidbody* thisPtr, RigidbodyFlag flags);
		static RigidbodyFlag Internal_getFlags(ScriptCRigidbody* thisPtr);
		static void Internal_addForce(ScriptCRigidbody* thisPtr, Vector3* force, ForceMode mode);
		static void Internal_addTorque(ScriptCRigidbody* thisPtr, Vector3* torque, ForceMode mode);
		static void Internal_addForceAtPoint(ScriptCRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode);
		static void Internal_getVelocityAtPoint(ScriptCRigidbody* thisPtr, Vector3* point, Vector3* __output);
	};
}
