//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptPhysicsQueryHit.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Physics. */
	class BS_SCR_BE_EXPORT ScriptPhysics : public ScriptObject<ScriptPhysics>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Physics")

	private:
		ScriptPhysics(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_GetGravity(Vector3* gravity);
		static void internal_SetGravity(Vector3* gravity);

		static int internal_AddPhysicsRegion(AABox* region);
		static void internal_RemovePhysicsRegion(int handle);
		static void internal_ClearPhysicsRegions();

		static void internal_ToggleCollision(UINT64 layerA, UINT64 layerB, bool enabled);
		static bool internal_IsCollisionEnabled(UINT64 layerA, UINT64 layerB);

		static bool internal_IsUpdateInProgress();

		static bool internal_RayCast(Vector3* origin, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max);
		static bool internal_BoxCast(AABox* box, Quaternion* rotation, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max);
		static bool internal_SphereCast(Sphere* sphere, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max);
		static bool internal_CapsuleCast(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max);
		static bool internal_ConvexCast(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max);

		static MonoArray* internal_RayCastAll(Vector3* origin, Vector3* unitDir, UINT64 layer, float max);
		static MonoArray* internal_BoxCastAll(AABox* box, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max);
		static MonoArray* internal_SphereCastAll(Sphere* sphere, Vector3* unitDir, UINT64 layer, float max);
		static MonoArray* internal_CapsuleCastAll(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max);
		static MonoArray* internal_ConvexCastAll(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max);

		static bool internal_RayCastAny(Vector3* origin, Vector3* unitDir, UINT64 layer, float max);
		static bool internal_BoxCastAny(AABox* box, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max);
		static bool internal_SphereCastAny(Sphere* sphere, Vector3* unitDir, UINT64 layer, float max);
		static bool internal_CapsuleCastAny(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max);
		static bool internal_ConvexCastAny(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max);

		static MonoArray* internal_BoxOverlap(AABox* box, Quaternion* rotation, UINT64 layer);
		static MonoArray* internal_SphereOverlap(Sphere* sphere, UINT64 layer);
		static MonoArray* internal_CapsuleOverlap(Capsule* capsule, Quaternion* rotation, UINT64 layer);
		static MonoArray* internal_ConvexOverlap(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, UINT64 layer);

		static bool internal_BoxOverlapAny(AABox* box, Quaternion* rotation, UINT64 layer);
		static bool internal_SphereOverlapAny(Sphere* sphere, UINT64 layer);
		static bool internal_CapsuleOverlapAny(Capsule* capsule, Quaternion* rotation, UINT64 layer);
		static bool internal_ConvexOverlapAny(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, UINT64 layer);
	};

	/** @} */
}