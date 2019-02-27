//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicsScene.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysics.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptPhysicsQueryHit.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"
#include "BsScriptCCollider.generated.h"

namespace bs
{
	ScriptPhysicsScene::ScriptPhysicsScene(MonoObject* managedInstance, const SPtr<PhysicsScene>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptPhysicsScene::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_rayCast", (void*)&ScriptPhysicsScene::Internal_rayCast);
		metaData.scriptClass->addInternalCall("Internal_rayCast0", (void*)&ScriptPhysicsScene::Internal_rayCast0);
		metaData.scriptClass->addInternalCall("Internal_boxCast", (void*)&ScriptPhysicsScene::Internal_boxCast);
		metaData.scriptClass->addInternalCall("Internal_sphereCast", (void*)&ScriptPhysicsScene::Internal_sphereCast);
		metaData.scriptClass->addInternalCall("Internal_capsuleCast", (void*)&ScriptPhysicsScene::Internal_capsuleCast);
		metaData.scriptClass->addInternalCall("Internal_convexCast", (void*)&ScriptPhysicsScene::Internal_convexCast);
		metaData.scriptClass->addInternalCall("Internal_rayCastAll", (void*)&ScriptPhysicsScene::Internal_rayCastAll);
		metaData.scriptClass->addInternalCall("Internal_rayCastAll0", (void*)&ScriptPhysicsScene::Internal_rayCastAll0);
		metaData.scriptClass->addInternalCall("Internal_boxCastAll", (void*)&ScriptPhysicsScene::Internal_boxCastAll);
		metaData.scriptClass->addInternalCall("Internal_sphereCastAll", (void*)&ScriptPhysicsScene::Internal_sphereCastAll);
		metaData.scriptClass->addInternalCall("Internal_capsuleCastAll", (void*)&ScriptPhysicsScene::Internal_capsuleCastAll);
		metaData.scriptClass->addInternalCall("Internal_convexCastAll", (void*)&ScriptPhysicsScene::Internal_convexCastAll);
		metaData.scriptClass->addInternalCall("Internal_rayCastAny", (void*)&ScriptPhysicsScene::Internal_rayCastAny);
		metaData.scriptClass->addInternalCall("Internal_rayCastAny0", (void*)&ScriptPhysicsScene::Internal_rayCastAny0);
		metaData.scriptClass->addInternalCall("Internal_boxCastAny", (void*)&ScriptPhysicsScene::Internal_boxCastAny);
		metaData.scriptClass->addInternalCall("Internal_sphereCastAny", (void*)&ScriptPhysicsScene::Internal_sphereCastAny);
		metaData.scriptClass->addInternalCall("Internal_capsuleCastAny", (void*)&ScriptPhysicsScene::Internal_capsuleCastAny);
		metaData.scriptClass->addInternalCall("Internal_convexCastAny", (void*)&ScriptPhysicsScene::Internal_convexCastAny);
		metaData.scriptClass->addInternalCall("Internal_boxOverlap", (void*)&ScriptPhysicsScene::Internal_boxOverlap);
		metaData.scriptClass->addInternalCall("Internal_sphereOverlap", (void*)&ScriptPhysicsScene::Internal_sphereOverlap);
		metaData.scriptClass->addInternalCall("Internal_capsuleOverlap", (void*)&ScriptPhysicsScene::Internal_capsuleOverlap);
		metaData.scriptClass->addInternalCall("Internal_convexOverlap", (void*)&ScriptPhysicsScene::Internal_convexOverlap);
		metaData.scriptClass->addInternalCall("Internal_boxOverlapAny", (void*)&ScriptPhysicsScene::Internal_boxOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_sphereOverlapAny", (void*)&ScriptPhysicsScene::Internal_sphereOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_capsuleOverlapAny", (void*)&ScriptPhysicsScene::Internal_capsuleOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_convexOverlapAny", (void*)&ScriptPhysicsScene::Internal_convexOverlapAny);
		metaData.scriptClass->addInternalCall("Internal_getGravity", (void*)&ScriptPhysicsScene::Internal_getGravity);
		metaData.scriptClass->addInternalCall("Internal_setGravity", (void*)&ScriptPhysicsScene::Internal_setGravity);
		metaData.scriptClass->addInternalCall("Internal_addBroadPhaseRegion", (void*)&ScriptPhysicsScene::Internal_addBroadPhaseRegion);
		metaData.scriptClass->addInternalCall("Internal_removeBroadPhaseRegion", (void*)&ScriptPhysicsScene::Internal_removeBroadPhaseRegion);
		metaData.scriptClass->addInternalCall("Internal_clearBroadPhaseRegions", (void*)&ScriptPhysicsScene::Internal_clearBroadPhaseRegions);

	}

	MonoObject* ScriptPhysicsScene::create(const SPtr<PhysicsScene>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptPhysicsScene>()) ScriptPhysicsScene(managedInstance, value);
		return managedInstance;
	}
	bool ScriptPhysicsScene::Internal_rayCast(ScriptPhysicsScene* thisPtr, Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->getInternal()->rayCast(*ray, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::Internal_rayCast0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->getInternal()->rayCast(*origin, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::Internal_boxCast(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->getInternal()->boxCast(*box, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::Internal_sphereCast(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->getInternal()->sphereCast(*sphere, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::Internal_capsuleCast(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->getInternal()->capsuleCast(*capsule, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::Internal_convexCast(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->getInternal()->convexCast(tmpmesh, *position, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::toInterop(tmphit);
		MonoUtil::valueCopy(hit, &interophit, ScriptPhysicsQueryHit::getMetaData()->scriptClass->_getInternalClass());

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_rayCastAll(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->getInternal()->rayCastAll(*ray, layer, max);

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

	MonoArray* ScriptPhysicsScene::Internal_rayCastAll0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->getInternal()->rayCastAll(*origin, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::Internal_boxCastAll(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->getInternal()->boxCastAll(*box, *rotation, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::Internal_sphereCastAll(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->getInternal()->sphereCastAll(*sphere, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::Internal_capsuleCastAll(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->getInternal()->capsuleCastAll(*capsule, *rotation, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::Internal_convexCastAll(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		vec__output = thisPtr->getInternal()->convexCastAll(tmpmesh, *position, *rotation, *unitDir, layer, max);

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

	bool ScriptPhysicsScene::Internal_rayCastAny(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->rayCastAny(*ray, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_rayCastAny0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->rayCastAny(*origin, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_boxCastAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->boxCastAny(*box, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_sphereCastAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->sphereCastAny(*sphere, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_capsuleCastAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->capsuleCastAny(*capsule, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_convexCastAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		tmp__output = thisPtr->getInternal()->convexCastAny(tmpmesh, *position, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_boxOverlap(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = thisPtr->getInternal()->boxOverlap(*box, *rotation, layer);

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

	MonoArray* ScriptPhysicsScene::Internal_sphereOverlap(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = thisPtr->getInternal()->sphereOverlap(*sphere, layer);

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

	MonoArray* ScriptPhysicsScene::Internal_capsuleOverlap(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = thisPtr->getInternal()->capsuleOverlap(*capsule, *rotation, layer);

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

	MonoArray* ScriptPhysicsScene::Internal_convexOverlap(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		vec__output = thisPtr->getInternal()->convexOverlap(tmpmesh, *position, *rotation, layer);

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

	bool ScriptPhysicsScene::Internal_boxOverlapAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->boxOverlapAny(*box, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_sphereOverlapAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->sphereOverlapAny(*sphere, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_capsuleOverlapAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->capsuleOverlapAny(*capsule, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_convexOverlapAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		tmp__output = thisPtr->getInternal()->convexOverlapAny(tmpmesh, *position, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::Internal_getGravity(ScriptPhysicsScene* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getInternal()->getGravity();

		*__output = tmp__output;
	}

	void ScriptPhysicsScene::Internal_setGravity(ScriptPhysicsScene* thisPtr, Vector3* gravity)
	{
		thisPtr->getInternal()->setGravity(*gravity);
	}

	uint32_t ScriptPhysicsScene::Internal_addBroadPhaseRegion(ScriptPhysicsScene* thisPtr, AABox* region)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->addBroadPhaseRegion(*region);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::Internal_removeBroadPhaseRegion(ScriptPhysicsScene* thisPtr, uint32_t handle)
	{
		thisPtr->getInternal()->removeBroadPhaseRegion(handle);
	}

	void ScriptPhysicsScene::Internal_clearBroadPhaseRegions(ScriptPhysicsScene* thisPtr)
	{
		thisPtr->getInternal()->clearBroadPhaseRegions();
	}
}
