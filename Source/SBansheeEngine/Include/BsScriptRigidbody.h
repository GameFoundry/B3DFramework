//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsRigidbody.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for Rigidbody. */
	class BS_SCR_BE_EXPORT ScriptRigidbody : public ScriptObject<ScriptRigidbody>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeRigidbody")

		/** Returns the native Rigidbody object. */
		Rigidbody* getRigidbody() const { return mRigidbody.get(); }

	private:
		friend class ScriptColliderBase;

		ScriptRigidbody(MonoObject* instance, const SPtr<Rigidbody>& body);

		/** Triggered when some object starts interacting with the rigidbody. */
		static void onCollisionBegin(MonoObject* instance, const CollisionData& collisionData);

		/** Triggered when some object remains interacting with the rigidbody throughout a frame. */
		static void onCollisionStay(MonoObject* instance, const CollisionData& collisionData);

		/** Triggered when some object ends interacting with the rigidbody. */
		static void onCollisionEnd(MonoObject* instance, const CollisionData& collisionData);

		SPtr<Rigidbody> mRigidbody;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptSceneObject* linkedSO);
		static void internal_Destroy(ScriptRigidbody* thisPtr);

		static void internal_Move(ScriptRigidbody* thisPtr, Vector3* position);
		static void internal_Rotate(ScriptRigidbody* thisPtr, Quaternion* rotation);

		static void internal_GetPosition(ScriptRigidbody* thisPtr, Vector3* position);
		static void internal_GetRotation(ScriptRigidbody* thisPtr, Quaternion* rotation);
		static void internal_SetTransform(ScriptRigidbody* thisPtr, Vector3* pos, Quaternion* rot);

		static void internal_SetMass(ScriptRigidbody* thisPtr, float mass);
		static float internal_GetMass(ScriptRigidbody* thisPtr);

		static void internal_SetIsKinematic(ScriptRigidbody* thisPtr, bool kinematic);
		static bool internal_GetIsKinematic(ScriptRigidbody* thisPtr);

		static bool internal_IsSleeping(ScriptRigidbody* thisPtr);
		static void internal_Sleep(ScriptRigidbody* thisPtr);
		static void internal_WakeUp(ScriptRigidbody* thisPtr);

		static void internal_SetSleepThreshold(ScriptRigidbody* thisPtr, float threshold);
		static float internal_GetSleepThreshold(ScriptRigidbody* thisPtr);

		static void internal_SetUseGravity(ScriptRigidbody* thisPtr, bool gravity);
		static bool internal_GetUseGravity(ScriptRigidbody* thisPtr);

		static void internal_SetVelocity(ScriptRigidbody* thisPtr, Vector3* velocity);
		static void internal_GetVelocity(ScriptRigidbody* thisPtr, Vector3* velocity);

		static void internal_SetAngularVelocity(ScriptRigidbody* thisPtr, Vector3* velocity);
		static void internal_GetAngularVelocity(ScriptRigidbody* thisPtr, Vector3* velocity);

		static void internal_SetDrag(ScriptRigidbody* thisPtr, float drag);
		static float internal_GetDrag(ScriptRigidbody* thisPtr);

		static void internal_SetAngularDrag(ScriptRigidbody* thisPtr, float drag);
		static float internal_GetAngularDrag(ScriptRigidbody* thisPtr);

		static void internal_SetInertiaTensor(ScriptRigidbody* thisPtr, Vector3* tensor);
		static void internal_GetInertiaTensor(ScriptRigidbody* thisPtr, Vector3* tensor);

		static void internal_SetMaxAngularVelocity(ScriptRigidbody* thisPtr, float maxVelocity);
		static float internal_GetMaxAngularVelocity(ScriptRigidbody* thisPtr);

		static void internal_SetCenterOfMass(ScriptRigidbody* thisPtr, Vector3* position, Quaternion* rotation);
		static void internal_GetCenterOfMassPosition(ScriptRigidbody* thisPtr, Vector3* position);
		static void internal_GetCenterOfMassRotation(ScriptRigidbody* thisPtr, Quaternion* rotation);

		static void internal_SetPositionSolverCount(ScriptRigidbody* thisPtr, UINT32 count);
		static UINT32 internal_GetPositionSolverCount(ScriptRigidbody* thisPtr);

		static void internal_SetVelocitySolverCount(ScriptRigidbody* thisPtr, UINT32 count);
		static UINT32 internal_GetVelocitySolverCount(ScriptRigidbody* thisPtr);

		static void internal_SetFlags(ScriptRigidbody* thisPtr, Rigidbody::Flag flags);
		static Rigidbody::Flag internal_GetFlags(ScriptRigidbody* thisPtr);

		static void internal_AddForce(ScriptRigidbody* thisPtr, Vector3* force, ForceMode mode);
		static void internal_AddTorque(ScriptRigidbody* thisPtr, Vector3* torque, ForceMode mode);
		static void internal_AddForceAtPoint(ScriptRigidbody* thisPtr, Vector3* force, Vector3* position, PointForceMode mode);

		static void internal_GetVelocityAtPoint(ScriptRigidbody* thisPtr, Vector3* point, Vector3* velocity);

		static void internal_AddCollider(ScriptRigidbody* thisPtr, ScriptColliderBase* collider);
		static void internal_RemoveCollider(ScriptRigidbody* thisPtr, ScriptColliderBase* collider);
		static void internal_RemoveColliders(ScriptRigidbody* thisPtr);

		static void internal_UpdateMassDistribution(ScriptRigidbody* thisPtr);

		typedef void(__stdcall *OnCollisionThunkDef) (MonoObject*, MonoObject*, MonoException**);

		static OnCollisionThunkDef onCollisionBeginThunk;
		static OnCollisionThunkDef onCollisionStayThunk;
		static OnCollisionThunkDef onCollisionEndThunk;
	};

	/** @} */
}