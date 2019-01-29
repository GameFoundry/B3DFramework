//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsRay.h"
#include "Math/BsVector3.h"
#include "Math/BsSphere.h"
#include "Math/BsAABox.h"
#include "Math/BsQuaternion.h"
#include "Math/BsCapsule.h"

namespace bs
{
	struct __PhysicsQueryHitInterop;
	class Physics;

	class BS_SCR_BE_EXPORT ScriptPhysics : public ScriptObject<ScriptPhysics>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Physics")

		ScriptPhysics(MonoObject* managedInstance);

	private:
		static bool Internal_rayCast(Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_rayCast0(Vector3* origin, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_boxCast(AABox* box, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_sphereCast(Sphere* sphere, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_capsuleCast(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_convexCast(MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static MonoArray* Internal_rayCastAll(Ray* ray, uint64_t layer, float max);
		static MonoArray* Internal_rayCastAll0(Vector3* origin, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_boxCastAll(AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_sphereCastAll(Sphere* sphere, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_capsuleCastAll(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_convexCastAll(MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_rayCastAny(Ray* ray, uint64_t layer, float max);
		static bool Internal_rayCastAny0(Vector3* origin, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_boxCastAny(AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_sphereCastAny(Sphere* sphere, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_capsuleCastAny(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_convexCastAny(MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_boxOverlap(AABox* box, Quaternion* rotation, uint64_t layer);
		static MonoArray* Internal_sphereOverlap(Sphere* sphere, uint64_t layer);
		static MonoArray* Internal_capsuleOverlap(Capsule* capsule, Quaternion* rotation, uint64_t layer);
		static MonoArray* Internal_convexOverlap(MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer);
		static bool Internal_boxOverlapAny(AABox* box, Quaternion* rotation, uint64_t layer);
		static bool Internal_sphereOverlapAny(Sphere* sphere, uint64_t layer);
		static bool Internal_capsuleOverlapAny(Capsule* capsule, Quaternion* rotation, uint64_t layer);
		static bool Internal_convexOverlapAny(MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer);
		static void Internal_getGravity(Vector3* __output);
		static void Internal_setGravity(Vector3* gravity);
		static uint32_t Internal_addBroadPhaseRegion(AABox* region);
		static void Internal_removeBroadPhaseRegion(uint32_t handle);
		static void Internal_clearBroadPhaseRegions();
		static void Internal_toggleCollision(uint64_t groupA, uint64_t groupB, bool enabled);
		static bool Internal_isCollisionEnabled(uint64_t groupA, uint64_t groupB);
		static bool Internal__isUpdateInProgress();
	};
}
