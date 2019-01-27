//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysics.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Physics/BsPhysics.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptPhysicsQueryHit.generated.h"
#include "../../../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"
#include "BsScriptCCollider.generated.h"

namespace bs
{
	ScriptPhysics::ScriptPhysics(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptPhysics::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_rayCast", (void*)&ScriptPhysics::Internal_rayCast);
		metaData.scriptClass->addInternalCall("Internal_rayCast0", (void*)&ScriptPhysics::Internal_rayCast0);
		metaData.scriptClass->addInternalCall("Internal_boxCast", (void*)&ScriptPhysics::Internal_boxCast);
		metaData.scriptClass->addInternalCall("Internal_sphereCast", (void*)&ScriptPhysics::Internal_sphereCast);
		metaData.scriptClass->addInternalCall("Internal_capsuleCast", (void*)&ScriptPhysics::Internal_capsuleCast);
		metaData.scriptClass->addInternalCall("Internal_convexCast", (void*)&ScriptPhysics::Internal_convexCast);
		metaData.scriptClass->addInternalCall("Internal_rayCastAll", (void*)&ScriptPhysics::Internal_rayCastAll);
		metaData.scriptClass->addInternalCall("Internal_rayCastAll0", (void*)&ScriptPhysics::Internal_rayCastAll0);
		metaData.scriptClass->addInternalCall("Internal_boxCastAll", (void*)&ScriptPhysics::Internal_boxCastAll);
		metaData.scriptClass->addInternalCall("Internal_sphereCastAll", (void*)&ScriptPhysics::Internal_sphereCastAll);
		metaData.scriptClass->addInternalCall("Internal_capsuleCastAll", (void*)&ScriptPhysics::Internal_capsuleCastAll);
		metaData.scriptClass->addInternalCall("Internal_convexCastAll", (void*)&ScriptPhysics::Internal_convexCastAll);
		metaData.scriptClass->addInternalCall("Internal_rayCastAny", (void*)&ScriptPhysics::Internal_rayCastAny);
		metaData.scriptClass->addInternalCall("Internal_rayCastAny0", (void*)&ScriptPhysics::Internal_rayCastAny0);
		metaData.scriptClass->addInternalCall("Internal_boxCastAny", (void*)&ScriptPhysics::Internal_boxCastAny);
		metaData.scriptClass->addInternalCall("Internal_sphereCastAny", (void*)&ScriptPhysics::Internal_sphereCastAny);
		metaData.scriptClass->addInternalCall("Internal_capsuleCastAny", (void*)&ScriptPhysics::Internal_capsuleCastAny);
		metaData.scriptClass->addInternalCall("Internal_convexCastAny", (void*)&ScriptPhysics::Internal_convexCastAny);
		metaData.scriptClass->addInternalCall("Internal_boxOverlap", (void*)&ScriptPhysics::Internal_boxOverlap);
		metaData.scriptClass->addInternalCall("Internal_sphereOverlap", (void*)&ScriptPhysics::Internal_sphereOverlap);
		metaData.scriptClass->addInternalCall("Internal_capsuleOverlap", (void*)&ScriptPhysics::Internal_capsuleOverlap);
		metaData.scriptClass->addInternalCall("Internal_convexOverlap", (void*)&ScriptPhysics::Internal_convexOverlap);
		metaData.scriptClass->addInternalCall("Internal_boxOverlapAny", (void*)&ScriptPhysics::Internal_boxOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_sphereOverlapAny", (void*)&ScriptPhysics::Internal_sphereOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_capsuleOverlapAny", (void*)&ScriptPhysics::Internal_capsuleOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_convexOverlapAny", (void*)&ScriptPhysics::Internal_convexOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_getGravity", (void*)&ScriptPhysics::Internal_getGravity);
		metaData.scriptClass->addInternalCall("Internal_setGravity", (void*)&ScriptPhysics::Internal_setGravity);
		metaData.scriptClass->addInternalCall("Internal_addBroadPhaseRegion", (void*)&ScriptPhysics::Internal_addBroadPhaseRegion);
		metaData.scriptClass->addInternalCall("Internal_removeBroadPhaseRegion", (void*)&ScriptPhysics::Internal_removeBroadPhaseRegion);
		metaData.scriptClass->addInternalCall("Internal_clearBroadPhaseRegions", (void*)&ScriptPhysics::Internal_clearBroadPhaseRegions);
		metaData.scriptClass->addInternalCall("Internal_toggleCollision", (void*)&ScriptPhysics::Internal_toggleCollision);
		metaData.scriptClass->addInternalCall("Internal_isCollisionEnabled", (void*)&ScriptPhysics::Internal_isCollisionEnabled);
		metaData.scriptClass->addInternalCall("Internal__isUpdateInProgress", (void*)&ScriptPhysics::Internal__isUpdateInProgress);

	}

	bool ScriptPhysics::Internal_rayCast(Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = Physics::instance().rayCast(*ray, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysics::Internal_rayCast0(Vector3* origin, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = Physics::instance().rayCast(*origin, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysics::Internal_boxCast(AABox* box, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = Physics::instance().boxCast(*box, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysics::Internal_sphereCast(Sphere* sphere, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = Physics::instance().sphereCast(*sphere, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysics::Internal_capsuleCast(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = Physics::instance().capsuleCast(*capsule, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysics::Internal_convexCast(MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		PhysicsQueryHit tmphit;
		tmp__output = Physics::instance().convexCast(tmpmesh, *position, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_rayCastAll(Ray* ray, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = Physics::instance().rayCastAll(*ray, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_rayCastAll0(Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = Physics::instance().rayCastAll(*origin, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_boxCastAll(AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = Physics::instance().boxCastAll(*box, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_sphereCastAll(Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = Physics::instance().sphereCastAll(*sphere, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_capsuleCastAll(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = Physics::instance().capsuleCastAll(*capsule, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_convexCastAll(MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		vec__output = Physics::instance().convexCastAll(tmpmesh, *position, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	bool ScriptPhysics::Internal_rayCastAny(Ray* ray, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = Physics::instance().rayCastAny(*ray, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_rayCastAny0(Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = Physics::instance().rayCastAny(*origin, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_boxCastAny(AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = Physics::instance().boxCastAny(*box, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_sphereCastAny(Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = Physics::instance().sphereCastAny(*sphere, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_capsuleCastAny(Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = Physics::instance().capsuleCastAny(*capsule, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_convexCastAny(MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		tmp__output = Physics::instance().convexCastAny(tmpmesh, *position, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_boxOverlap(AABox* box, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = Physics::instance().boxOverlap(*box, *rotation, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().getBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.set(i, script__output->getManagedInstance());
			else
				array__output.set(i, nullptr);
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_sphereOverlap(Sphere* sphere, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = Physics::instance().sphereOverlap(*sphere, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().getBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.set(i, script__output->getManagedInstance());
			else
				array__output.set(i, nullptr);
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_capsuleOverlap(Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = Physics::instance().capsuleOverlap(*capsule, *rotation, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().getBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.set(i, script__output->getManagedInstance());
			else
				array__output.set(i, nullptr);
		}
		__output = array__output.getInternal();

		return __output;
	}

	MonoArray* ScriptPhysics::Internal_convexOverlap(MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		vec__output = Physics::instance().convexOverlap(tmpmesh, *position, *rotation, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().getBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.set(i, script__output->getManagedInstance());
			else
				array__output.set(i, nullptr);
		}
		__output = array__output.getInternal();

		return __output;
	}

	bool ScriptPhysics::Internal_boxOverlapAny(AABox* box, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = Physics::instance().boxOverlapAny(*box, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_sphereOverlapAny(Sphere* sphere, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = Physics::instance().sphereOverlapAny(*sphere, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_capsuleOverlapAny(Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = Physics::instance().capsuleOverlapAny(*capsule, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal_convexOverlapAny(MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		tmp__output = Physics::instance().convexOverlapAny(tmpmesh, *position, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysics::Internal_getGravity(Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = Physics::instance().getGravity();

		*__output = tmp__output;
	}

	void ScriptPhysics::Internal_setGravity(Vector3* gravity)
	{
		Physics::instance().setGravity(*gravity);
	}

	uint32_t ScriptPhysics::Internal_addBroadPhaseRegion(AABox* region)
	{
		uint32_t tmp__output;
		tmp__output = Physics::instance().addBroadPhaseRegion(*region);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysics::Internal_removeBroadPhaseRegion(uint32_t handle)
	{
		Physics::instance().removeBroadPhaseRegion(handle);
	}

	void ScriptPhysics::Internal_clearBroadPhaseRegions()
	{
		Physics::instance().clearBroadPhaseRegions();
	}

	void ScriptPhysics::Internal_toggleCollision(uint64_t groupA, uint64_t groupB, bool enabled)
	{
		Physics::instance().toggleCollision(groupA, groupB, enabled);
	}

	bool ScriptPhysics::Internal_isCollisionEnabled(uint64_t groupA, uint64_t groupB)
	{
		bool tmp__output;
		tmp__output = Physics::instance().isCollisionEnabled(groupA, groupB);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal__isUpdateInProgress()
	{
		bool tmp__output;
		tmp__output = Physics::instance()._isUpdateInProgress();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
