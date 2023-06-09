//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "Math/BsRay.h"
#include "Math/BsSphere.h"
#include "Math/BsAABox.h"
#include "Math/BsQuaternion.h"
#include "Math/BsCapsule.h"

namespace bs { struct __PhysicsQueryHitInterop; }
namespace bs { class PhysicsScene; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysicsScene : public ScriptObject<ScriptPhysicsScene>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PhysicsScene")

		ScriptPhysicsScene(MonoObject* managedInstance, const SPtr<PhysicsScene>& value);

		SPtr<PhysicsScene> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<PhysicsScene>& value);

	private:
		SPtr<PhysicsScene> mInternal;

		static bool InternalRayCast(ScriptPhysicsScene* thisPtr, Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalRayCast0(ScriptPhysicsScene* thisPtr, TVector3<float>* origin, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalBoxCast(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalSphereCast(ScriptPhysicsScene* thisPtr, Sphere* sphere, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalCapsuleCast(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalConvexCast(ScriptPhysicsScene* thisPtr, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static MonoArray* InternalRayCastAll(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max);
		static MonoArray* InternalRayCastAll0(ScriptPhysicsScene* thisPtr, TVector3<float>* origin, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalBoxCastAll(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalSphereCastAll(ScriptPhysicsScene* thisPtr, Sphere* sphere, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalCapsuleCastAll(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalConvexCastAll(ScriptPhysicsScene* thisPtr, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalRayCastAny(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max);
		static bool InternalRayCastAny0(ScriptPhysicsScene* thisPtr, TVector3<float>* origin, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalBoxCastAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalSphereCastAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalCapsuleCastAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalConvexCastAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalBoxOverlap(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer);
		static MonoArray* InternalSphereOverlap(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer);
		static MonoArray* InternalCapsuleOverlap(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer);
		static MonoArray* InternalConvexOverlap(ScriptPhysicsScene* thisPtr, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, uint64_t layer);
		static bool InternalBoxOverlapAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer);
		static bool InternalSphereOverlapAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer);
		static bool InternalCapsuleOverlapAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer);
		static bool InternalConvexOverlapAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, uint64_t layer);
		static void InternalGetGravity(ScriptPhysicsScene* thisPtr, TVector3<float>* __output);
		static void InternalSetGravity(ScriptPhysicsScene* thisPtr, TVector3<float>* gravity);
		static uint32_t InternalAddBroadPhaseRegion(ScriptPhysicsScene* thisPtr, AABox* region);
		static void InternalRemoveBroadPhaseRegion(ScriptPhysicsScene* thisPtr, uint32_t handle);
		static void InternalClearBroadPhaseRegions(ScriptPhysicsScene* thisPtr);
	};
}
