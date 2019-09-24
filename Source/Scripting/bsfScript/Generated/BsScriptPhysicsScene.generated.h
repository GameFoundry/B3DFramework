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

namespace bs { struct __PhysicsQueryHitInterop; }
namespace bs { class PhysicsScene; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPhysicsScene : public ScriptObject<ScriptPhysicsScene>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "PhysicsScene")

		ScriptPhysicsScene(MonoObject* managedInstance, const SPtr<PhysicsScene>& value);

		SPtr<PhysicsScene> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<PhysicsScene>& value);

	private:
		SPtr<PhysicsScene> mInternal;

		static bool Internal_rayCast(ScriptPhysicsScene* thisPtr, Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_rayCast0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_boxCast(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_sphereCast(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_capsuleCast(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool Internal_convexCast(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static MonoArray* Internal_rayCastAll(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max);
		static MonoArray* Internal_rayCastAll0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_boxCastAll(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_sphereCastAll(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_capsuleCastAll(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_convexCastAll(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_rayCastAny(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max);
		static bool Internal_rayCastAny0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_boxCastAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_sphereCastAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_capsuleCastAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static bool Internal_convexCastAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max);
		static MonoArray* Internal_boxOverlap(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer);
		static MonoArray* Internal_sphereOverlap(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer);
		static MonoArray* Internal_capsuleOverlap(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer);
		static MonoArray* Internal_convexOverlap(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer);
		static bool Internal_boxOverlapAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer);
		static bool Internal_sphereOverlapAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer);
		static bool Internal_capsuleOverlapAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer);
		static bool Internal_convexOverlapAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer);
		static void Internal_getGravity(ScriptPhysicsScene* thisPtr, Vector3* __output);
		static void Internal_setGravity(ScriptPhysicsScene* thisPtr, Vector3* gravity);
		static uint32_t Internal_addBroadPhaseRegion(ScriptPhysicsScene* thisPtr, AABox* region);
		static void Internal_removeBroadPhaseRegion(ScriptPhysicsScene* thisPtr, uint32_t handle);
		static void Internal_clearBroadPhaseRegions(ScriptPhysicsScene* thisPtr);
	};
}
