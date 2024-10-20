//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysics.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsRay.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "../../../Foundation/bsfUtility/Math/BsAABox.h"
#include "../../../Foundation/bsfUtility/Math/BsQuaternion.h"
#include "../../../Foundation/bsfUtility/Math/BsSphere.h"
#include "Math/BsCapsule.h"

namespace bs { struct __TRay_float_Interop; }
namespace bs { struct __TSphere_float_Interop; }
namespace bs { struct __PhysicsQueryHitInterop; }
namespace bs { struct __TAABox_float_Interop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysicsScene : public TScriptNonReflectableWrapper<PhysicsScene, ScriptPhysicsScene>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PhysicsScene")

		ScriptPhysicsScene(const SPtr<PhysicsScene>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static bool InternalRayCast(ScriptPhysicsScene* self, __TRay_float_Interop* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalRayCast0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalBoxCast(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalSphereCast(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalCapsuleCast(ScriptPhysicsScene* self, Capsule* capsule, TQuaternion<float>* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static bool InternalConvexCast(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max);
		static MonoArray* InternalRayCastAll(ScriptPhysicsScene* self, __TRay_float_Interop* ray, uint64_t layer, float max);
		static MonoArray* InternalRayCastAll0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalBoxCastAll(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalSphereCastAll(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalCapsuleCastAll(ScriptPhysicsScene* self, Capsule* capsule, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalConvexCastAll(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalRayCastAny(ScriptPhysicsScene* self, __TRay_float_Interop* ray, uint64_t layer, float max);
		static bool InternalRayCastAny0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalBoxCastAny(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalSphereCastAny(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalCapsuleCastAny(ScriptPhysicsScene* self, Capsule* capsule, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static bool InternalConvexCastAny(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max);
		static MonoArray* InternalBoxOverlap(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, uint64_t layer);
		static MonoArray* InternalSphereOverlap(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, uint64_t layer);
		static MonoArray* InternalCapsuleOverlap(ScriptPhysicsScene* self, Capsule* capsule, TQuaternion<float>* rotation, uint64_t layer);
		static MonoArray* InternalConvexOverlap(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, uint64_t layer);
		static bool InternalBoxOverlapAny(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, uint64_t layer);
		static bool InternalSphereOverlapAny(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, uint64_t layer);
		static bool InternalCapsuleOverlapAny(ScriptPhysicsScene* self, Capsule* capsule, TQuaternion<float>* rotation, uint64_t layer);
		static bool InternalConvexOverlapAny(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, uint64_t layer);
		static void InternalGetGravity(ScriptPhysicsScene* self, TVector3<float>* __output);
		static void InternalSetGravity(ScriptPhysicsScene* self, TVector3<float>* gravity);
		static uint32_t InternalAddBroadPhaseRegion(ScriptPhysicsScene* self, __TAABox_float_Interop* region);
		static void InternalRemoveBroadPhaseRegion(ScriptPhysicsScene* self, uint32_t handle);
		static void InternalClearBroadPhaseRegions(ScriptPhysicsScene* self);
	};
}
