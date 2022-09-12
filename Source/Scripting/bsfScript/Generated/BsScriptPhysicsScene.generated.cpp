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

	void ScriptPhysicsScene::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_rayCast", (void*)&ScriptPhysicsScene::Internal_rayCast);
		metaData.scriptClass->AddInternalCall("Internal_rayCast0", (void*)&ScriptPhysicsScene::Internal_rayCast0);
		metaData.scriptClass->AddInternalCall("Internal_boxCast", (void*)&ScriptPhysicsScene::Internal_boxCast);
		metaData.scriptClass->AddInternalCall("Internal_sphereCast", (void*)&ScriptPhysicsScene::Internal_sphereCast);
		metaData.scriptClass->AddInternalCall("Internal_capsuleCast", (void*)&ScriptPhysicsScene::Internal_capsuleCast);
		metaData.scriptClass->AddInternalCall("Internal_convexCast", (void*)&ScriptPhysicsScene::Internal_convexCast);
		metaData.scriptClass->AddInternalCall("Internal_rayCastAll", (void*)&ScriptPhysicsScene::Internal_rayCastAll);
		metaData.scriptClass->AddInternalCall("Internal_rayCastAll0", (void*)&ScriptPhysicsScene::Internal_rayCastAll0);
		metaData.scriptClass->AddInternalCall("Internal_boxCastAll", (void*)&ScriptPhysicsScene::Internal_boxCastAll);
		metaData.scriptClass->AddInternalCall("Internal_sphereCastAll", (void*)&ScriptPhysicsScene::Internal_sphereCastAll);
		metaData.scriptClass->AddInternalCall("Internal_capsuleCastAll", (void*)&ScriptPhysicsScene::Internal_capsuleCastAll);
		metaData.scriptClass->AddInternalCall("Internal_convexCastAll", (void*)&ScriptPhysicsScene::Internal_convexCastAll);
		metaData.scriptClass->AddInternalCall("Internal_rayCastAny", (void*)&ScriptPhysicsScene::Internal_rayCastAny);
		metaData.scriptClass->AddInternalCall("Internal_rayCastAny0", (void*)&ScriptPhysicsScene::Internal_rayCastAny0);
		metaData.scriptClass->AddInternalCall("Internal_boxCastAny", (void*)&ScriptPhysicsScene::Internal_boxCastAny);
		metaData.scriptClass->AddInternalCall("Internal_sphereCastAny", (void*)&ScriptPhysicsScene::Internal_sphereCastAny);
		metaData.scriptClass->AddInternalCall("Internal_capsuleCastAny", (void*)&ScriptPhysicsScene::Internal_capsuleCastAny);
		metaData.scriptClass->AddInternalCall("Internal_convexCastAny", (void*)&ScriptPhysicsScene::Internal_convexCastAny);
		metaData.scriptClass->AddInternalCall("Internal_boxOverlap", (void*)&ScriptPhysicsScene::Internal_boxOverlap);
		metaData.scriptClass->AddInternalCall("Internal_sphereOverlap", (void*)&ScriptPhysicsScene::Internal_sphereOverlap);
		metaData.scriptClass->AddInternalCall("Internal_capsuleOverlap", (void*)&ScriptPhysicsScene::Internal_capsuleOverlap);
		metaData.scriptClass->AddInternalCall("Internal_convexOverlap", (void*)&ScriptPhysicsScene::Internal_convexOverlap);
		metaData.scriptClass->AddInternalCall("Internal_boxOverlapAny", (void*)&ScriptPhysicsScene::Internal_boxOverlapAny);
		metaData.scriptClass->AddInternalCall("Internal_sphereOverlapAny", (void*)&ScriptPhysicsScene::Internal_sphereOverlapAny);
		metaData.scriptClass->AddInternalCall("Internal_capsuleOverlapAny", (void*)&ScriptPhysicsScene::Internal_capsuleOverlapAny);
		metaData.scriptClass->AddInternalCall("Internal_convexOverlapAny", (void*)&ScriptPhysicsScene::Internal_convexOverlapAny);
		metaData.scriptClass->AddInternalCall("Internal_getGravity", (void*)&ScriptPhysicsScene::Internal_getGravity);
		metaData.scriptClass->AddInternalCall("Internal_setGravity", (void*)&ScriptPhysicsScene::Internal_setGravity);
		metaData.scriptClass->AddInternalCall("Internal_addBroadPhaseRegion", (void*)&ScriptPhysicsScene::Internal_addBroadPhaseRegion);
		metaData.scriptClass->AddInternalCall("Internal_removeBroadPhaseRegion", (void*)&ScriptPhysicsScene::Internal_removeBroadPhaseRegion);
		metaData.scriptClass->AddInternalCall("Internal_clearBroadPhaseRegions", (void*)&ScriptPhysicsScene::Internal_clearBroadPhaseRegions);

	}

	MonoObject* ScriptPhysicsScene::create(const SPtr<PhysicsScene>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptPhysicsScene>()) ScriptPhysicsScene(managedInstance, value);
		return managedInstance;
	}
	bool ScriptPhysicsScene::Internal_rayCast(ScriptPhysicsScene* thisPtr, Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->GetInternal()->rayCast(*ray, tmphit, layer, max);

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
		tmp__output = thisPtr->GetInternal()->rayCast(*origin, *unitDir, tmphit, layer, max);

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
		tmp__output = thisPtr->GetInternal()->boxCast(*box, *rotation, *unitDir, tmphit, layer, max);

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
		tmp__output = thisPtr->GetInternal()->sphereCast(*sphere, *unitDir, tmphit, layer, max);

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
		tmp__output = thisPtr->GetInternal()->capsuleCast(*capsule, *rotation, *unitDir, tmphit, layer, max);

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
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
		PhysicsQueryHit tmphit;
		tmp__output = thisPtr->GetInternal()->convexCast(tmpmesh, *position, *rotation, *unitDir, tmphit, layer, max);

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
		vec__output = thisPtr->GetInternal()->rayCastAll(*ray, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_rayCastAll0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->GetInternal()->rayCastAll(*origin, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_boxCastAll(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->GetInternal()->boxCastAll(*box, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_sphereCastAll(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->GetInternal()->sphereCastAll(*sphere, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_capsuleCastAll(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		vec__output = thisPtr->GetInternal()->capsuleCastAll(*capsule, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_convexCastAll(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> vec__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
		vec__output = thisPtr->GetInternal()->convexCastAll(tmpmesh, *position, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptPhysicsQueryHit>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptPhysicsQueryHit::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	bool ScriptPhysicsScene::Internal_rayCastAny(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->rayCastAny(*ray, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_rayCastAny0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->rayCastAny(*origin, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_boxCastAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->boxCastAny(*box, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_sphereCastAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->sphereCastAny(*sphere, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_capsuleCastAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->capsuleCastAny(*capsule, *rotation, *unitDir, layer, max);

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
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
		tmp__output = thisPtr->GetInternal()->convexCastAny(tmpmesh, *position, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_boxOverlap(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = thisPtr->GetInternal()->boxOverlap(*box, *rotation, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_sphereOverlap(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = thisPtr->GetInternal()->sphereOverlap(*sphere, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_capsuleOverlap(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		vec__output = thisPtr->GetInternal()->capsuleOverlap(*capsule, *rotation, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::Internal_convexOverlap(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> vec__output;
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
		vec__output = thisPtr->GetInternal()->convexOverlap(tmpmesh, *position, *rotation, layer);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptCCollider>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptComponentBase* script__output = nullptr;
			if(vec__output[i])
				script__output = ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	bool ScriptPhysicsScene::Internal_boxOverlapAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->boxOverlapAny(*box, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_sphereOverlapAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->sphereOverlapAny(*sphere, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::Internal_capsuleOverlapAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->capsuleOverlapAny(*capsule, *rotation, layer);

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
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
		tmp__output = thisPtr->GetInternal()->convexOverlapAny(tmpmesh, *position, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::Internal_getGravity(ScriptPhysicsScene* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->getGravity();

		*__output = tmp__output;
	}

	void ScriptPhysicsScene::Internal_setGravity(ScriptPhysicsScene* thisPtr, Vector3* gravity)
	{
		thisPtr->GetInternal()->setGravity(*gravity);
	}

	uint32_t ScriptPhysicsScene::Internal_addBroadPhaseRegion(ScriptPhysicsScene* thisPtr, AABox* region)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->addBroadPhaseRegion(*region);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::Internal_removeBroadPhaseRegion(ScriptPhysicsScene* thisPtr, uint32_t handle)
	{
		thisPtr->GetInternal()->removeBroadPhaseRegion(handle);
	}

	void ScriptPhysicsScene::Internal_clearBroadPhaseRegions(ScriptPhysicsScene* thisPtr)
	{
		thisPtr->GetInternal()->clearBroadPhaseRegions();
	}
}
