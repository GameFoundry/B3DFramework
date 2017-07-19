//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPhysics.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsPhysics.h"
#include "BsScriptCollider.h"
#include "BsCollider.h"

#include "BsScriptPhysicsMesh.generated.h"

namespace bs
{
	ScriptPhysics::ScriptPhysics(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPhysics::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetGravity", &ScriptPhysics::internal_GetGravity);
		metaData.scriptClass->addInternalCall("Internal_SetGravity", &ScriptPhysics::internal_SetGravity);

		metaData.scriptClass->addInternalCall("Internal_AddPhysicsRegion", &ScriptPhysics::internal_AddPhysicsRegion);
		metaData.scriptClass->addInternalCall("Internal_RemovePhysicsRegion", &ScriptPhysics::internal_RemovePhysicsRegion);
		metaData.scriptClass->addInternalCall("Internal_ClearPhysicsRegions", &ScriptPhysics::internal_ClearPhysicsRegions);

		metaData.scriptClass->addInternalCall("Internal_ToggleCollision", &ScriptPhysics::internal_ToggleCollision);
		metaData.scriptClass->addInternalCall("Internal_IsCollisionEnabled", &ScriptPhysics::internal_IsCollisionEnabled);

		metaData.scriptClass->addInternalCall("Internal_IsUpdateInProgress", &ScriptPhysics::internal_IsUpdateInProgress);

		metaData.scriptClass->addInternalCall("Internal_RayCast", &ScriptPhysics::internal_RayCast);
		metaData.scriptClass->addInternalCall("Internal_BoxCast", &ScriptPhysics::internal_BoxCast);
		metaData.scriptClass->addInternalCall("Internal_SphereCast", &ScriptPhysics::internal_SphereCast);
		metaData.scriptClass->addInternalCall("Internal_CapsuleCast", &ScriptPhysics::internal_CapsuleCast);
		metaData.scriptClass->addInternalCall("Internal_ConvexCast", &ScriptPhysics::internal_ConvexCast);

		metaData.scriptClass->addInternalCall("Internal_RayCastAll", &ScriptPhysics::internal_RayCastAll);
		metaData.scriptClass->addInternalCall("Internal_BoxCastAll", &ScriptPhysics::internal_BoxCastAll);
		metaData.scriptClass->addInternalCall("Internal_SphereCastAll", &ScriptPhysics::internal_SphereCastAll);
		metaData.scriptClass->addInternalCall("Internal_CapsuleCastAll", &ScriptPhysics::internal_CapsuleCastAll);
		metaData.scriptClass->addInternalCall("Internal_ConvexCastAll", &ScriptPhysics::internal_ConvexCastAll);

		metaData.scriptClass->addInternalCall("Internal_RayCastAny", &ScriptPhysics::internal_RayCastAny);
		metaData.scriptClass->addInternalCall("Internal_BoxCastAny", &ScriptPhysics::internal_BoxCastAny);
		metaData.scriptClass->addInternalCall("Internal_SphereCastAny", &ScriptPhysics::internal_SphereCastAny);
		metaData.scriptClass->addInternalCall("Internal_CapsuleCastAny", &ScriptPhysics::internal_CapsuleCastAny);
		metaData.scriptClass->addInternalCall("Internal_ConvexCastAny", &ScriptPhysics::internal_ConvexCastAny);

		metaData.scriptClass->addInternalCall("Internal_BoxOverlap", &ScriptPhysics::internal_BoxOverlap);
		metaData.scriptClass->addInternalCall("Internal_SphereOverlap", &ScriptPhysics::internal_SphereOverlap);
		metaData.scriptClass->addInternalCall("Internal_CapsuleOverlap", &ScriptPhysics::internal_CapsuleOverlap);
		metaData.scriptClass->addInternalCall("Internal_ConvexOverlap", &ScriptPhysics::internal_ConvexOverlap);

		metaData.scriptClass->addInternalCall("Internal_BoxOverlapAny", &ScriptPhysics::internal_BoxOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_SphereOverlapAny", &ScriptPhysics::internal_SphereOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_CapsuleOverlapAny", &ScriptPhysics::internal_CapsuleOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_ConvexOverlapAny", &ScriptPhysics::internal_ConvexOverlapAny);
	}

	void ScriptPhysics::internal_GetGravity(Vector3* gravity)
	{
		*gravity = gPhysics().getGravity();
	}

	void ScriptPhysics::internal_SetGravity(Vector3* gravity)
	{
		gPhysics().setGravity(*gravity);
	}

	int ScriptPhysics::internal_AddPhysicsRegion(AABox* region)
	{
		return gPhysics().addBroadPhaseRegion(*region);
	}

	void ScriptPhysics::internal_RemovePhysicsRegion(int handle)
	{
		gPhysics().removeBroadPhaseRegion(handle);
	}

	void ScriptPhysics::internal_ClearPhysicsRegions()
	{
		gPhysics().clearBroadPhaseRegions();
	}

	void ScriptPhysics::internal_ToggleCollision(UINT64 layerA, UINT64 layerB, bool enabled)
	{
		gPhysics().toggleCollision(layerA, layerB, enabled);
	}

	bool ScriptPhysics::internal_IsCollisionEnabled(UINT64 layerA, UINT64 layerB)
	{
		return gPhysics().isCollisionEnabled(layerA, layerB);
	}

	bool ScriptPhysics::internal_IsUpdateInProgress()
	{
		return gPhysics()._isUpdateInProgress();
	}

	bool ScriptPhysics::internal_RayCast(Vector3* origin, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max)
	{
		PhysicsQueryHit nativeHit;
		bool wasHit = gPhysics().rayCast(*origin, *unitDir, nativeHit, layer, max);
		*hit = ScriptPhysicsQueryHitHelper::create(nativeHit);

		return wasHit;
	}

	bool ScriptPhysics::internal_BoxCast(AABox* box, Quaternion* rotation, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max)
	{
		PhysicsQueryHit nativeHit;
		bool wasHit = gPhysics().boxCast(*box, *rotation, *unitDir, nativeHit, layer, max);
		*hit = ScriptPhysicsQueryHitHelper::create(nativeHit);

		return wasHit;
	}

	bool ScriptPhysics::internal_SphereCast(Sphere* sphere, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max)
	{
		PhysicsQueryHit nativeHit;
		bool wasHit = gPhysics().sphereCast(*sphere, *unitDir, nativeHit, layer, max);
		*hit = ScriptPhysicsQueryHitHelper::create(nativeHit);

		return wasHit;
	}

	bool ScriptPhysics::internal_CapsuleCast(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max)
	{
		PhysicsQueryHit nativeHit;
		bool wasHit = gPhysics().capsuleCast(*capsule, *rotation, *unitDir, nativeHit, layer, max);
		*hit = ScriptPhysicsQueryHitHelper::create(nativeHit);

		return wasHit;
	}

	bool ScriptPhysics::internal_ConvexCast(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, ScriptPhysicsQueryHit* hit, UINT64 layer, float max)
	{
		HPhysicsMesh meshHandle;
		if (mesh != nullptr)
			meshHandle = mesh->getHandle();

		PhysicsQueryHit nativeHit;
		bool wasHit = gPhysics().convexCast(meshHandle, *position, *rotation, *unitDir, nativeHit, layer, max);
		*hit = ScriptPhysicsQueryHitHelper::create(nativeHit);

		return wasHit;
	}

	MonoArray* nativeToManagedHitArray(const Vector<PhysicsQueryHit>& hits)
	{
		UINT32 numHits = (UINT32)hits.size();
		ScriptArray output = ScriptArray::create<ScriptPhysicsQueryHitHelper>(numHits);

		for (UINT32 i = 0; i < numHits; i++)
			output.set(i, ScriptPhysicsQueryHitHelper::create(hits[i]));

		return output.getInternal();
	}

	MonoArray* ScriptPhysics::internal_RayCastAll(Vector3* origin, Vector3* unitDir, UINT64 layer, float max)
	{
		return nativeToManagedHitArray(gPhysics().rayCastAll(*origin, *unitDir, layer, max));
	}

	MonoArray* ScriptPhysics::internal_BoxCastAll(AABox* box, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max)
	{
		return nativeToManagedHitArray(gPhysics().boxCastAll(*box, *rotation, *unitDir, layer, max));
	}

	MonoArray* ScriptPhysics::internal_SphereCastAll(Sphere* sphere, Vector3* unitDir, UINT64 layer, float max)
	{
		return nativeToManagedHitArray(gPhysics().sphereCastAll(*sphere, *unitDir, layer, max));
	}

	MonoArray* ScriptPhysics::internal_CapsuleCastAll(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max)
	{
		return nativeToManagedHitArray(gPhysics().capsuleCastAll(*capsule, *rotation, *unitDir, layer, max));
	}

	MonoArray* ScriptPhysics::internal_ConvexCastAll(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max)
	{
		HPhysicsMesh meshHandle;
		if (mesh != nullptr)
			meshHandle = mesh->getHandle();

		return nativeToManagedHitArray(gPhysics().convexCastAll(meshHandle, *position, *rotation, *unitDir, layer, max));
	}

	bool ScriptPhysics::internal_RayCastAny(Vector3* origin, Vector3* unitDir, UINT64 layer, float max)
	{
		return gPhysics().rayCastAny(*origin, *unitDir, layer, max);
	}

	bool ScriptPhysics::internal_BoxCastAny(AABox* box, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max)
	{
		return gPhysics().boxCastAny(*box, *rotation, *unitDir, layer, max);
	}

	bool ScriptPhysics::internal_SphereCastAny(Sphere* sphere, Vector3* unitDir, UINT64 layer, float max)
	{
		return gPhysics().sphereCastAny(*sphere, *unitDir, layer, max);
	}

	bool ScriptPhysics::internal_CapsuleCastAny(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max)
	{
		return gPhysics().capsuleCastAny(*capsule, *rotation, *unitDir, layer, max);
	}

	bool ScriptPhysics::internal_ConvexCastAny(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, UINT64 layer, float max)
	{
		HPhysicsMesh meshHandle;
		if (mesh != nullptr)
			meshHandle = mesh->getHandle();

		return gPhysics().convexCastAny(meshHandle, *position, *rotation, *unitDir, layer, max);
	}

	MonoArray* nativeToManagedColliderArray(const Vector<Collider*>& colliders)
	{
		UINT32 numColliders = (UINT32)colliders.size();
		ScriptArray output = ScriptArray::create<ScriptCollider>(numColliders);
		for (UINT32 i = 0; i < numColliders; i++)
			output.set(i, colliders[i]->_getOwner(PhysicsOwnerType::Script));

		return output.getInternal();
	}

	MonoArray* ScriptPhysics::internal_BoxOverlap(AABox* box, Quaternion* rotation, UINT64 layer)
	{
		return nativeToManagedColliderArray(gPhysics()._boxOverlap(*box, *rotation, layer));
	}

	MonoArray* ScriptPhysics::internal_SphereOverlap(Sphere* sphere, UINT64 layer)
	{
		return nativeToManagedColliderArray(gPhysics()._sphereOverlap(*sphere, layer));
	}

	MonoArray* ScriptPhysics::internal_CapsuleOverlap(Capsule* capsule, Quaternion* rotation, UINT64 layer)
	{
		return nativeToManagedColliderArray(gPhysics()._capsuleOverlap(*capsule, *rotation, layer));
	}

	MonoArray* ScriptPhysics::internal_ConvexOverlap(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, UINT64 layer)
	{
		HPhysicsMesh meshHandle;
		if (mesh != nullptr)
			meshHandle = mesh->getHandle();

		return nativeToManagedColliderArray(gPhysics()._convexOverlap(meshHandle, *position, *rotation, layer));
	}

	bool ScriptPhysics::internal_BoxOverlapAny(AABox* box, Quaternion* rotation, UINT64 layer)
	{
		return gPhysics().boxOverlapAny(*box, *rotation, layer);
	}

	bool ScriptPhysics::internal_SphereOverlapAny(Sphere* sphere, UINT64 layer)
	{
		return gPhysics().sphereOverlapAny(*sphere, layer);
	}

	bool ScriptPhysics::internal_CapsuleOverlapAny(Capsule* capsule, Quaternion* rotation, UINT64 layer)
	{
		return gPhysics().capsuleOverlapAny(*capsule, *rotation, layer);
	}

	bool ScriptPhysics::internal_ConvexOverlapAny(ScriptPhysicsMesh* mesh, Vector3* position, Quaternion* rotation, UINT64 layer)
	{
		HPhysicsMesh meshHandle;
		if (mesh != nullptr)
			meshHandle = mesh->getHandle();

		return gPhysics().convexOverlapAny(meshHandle, *position, *rotation, layer);
	}
}