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

using namespace bs;
ScriptPhysicsScene::ScriptPhysicsScene(MonoObject* managedInstance, const SPtr<PhysicsScene>& value)
	: ScriptObject(managedInstance), mInternal(value)
{
}

void ScriptPhysicsScene::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_RayCast", (void*)&ScriptPhysicsScene::InternalRayCast);
	metaData.ScriptClass->AddInternalCall("Internal_RayCast0", (void*)&ScriptPhysicsScene::InternalRayCast0);
	metaData.ScriptClass->AddInternalCall("Internal_BoxCast", (void*)&ScriptPhysicsScene::InternalBoxCast);
	metaData.ScriptClass->AddInternalCall("Internal_SphereCast", (void*)&ScriptPhysicsScene::InternalSphereCast);
	metaData.ScriptClass->AddInternalCall("Internal_CapsuleCast", (void*)&ScriptPhysicsScene::InternalCapsuleCast);
	metaData.ScriptClass->AddInternalCall("Internal_ConvexCast", (void*)&ScriptPhysicsScene::InternalConvexCast);
	metaData.ScriptClass->AddInternalCall("Internal_RayCastAll", (void*)&ScriptPhysicsScene::InternalRayCastAll);
	metaData.ScriptClass->AddInternalCall("Internal_RayCastAll0", (void*)&ScriptPhysicsScene::InternalRayCastAll0);
	metaData.ScriptClass->AddInternalCall("Internal_BoxCastAll", (void*)&ScriptPhysicsScene::InternalBoxCastAll);
	metaData.ScriptClass->AddInternalCall("Internal_SphereCastAll", (void*)&ScriptPhysicsScene::InternalSphereCastAll);
	metaData.ScriptClass->AddInternalCall("Internal_CapsuleCastAll", (void*)&ScriptPhysicsScene::InternalCapsuleCastAll);
	metaData.ScriptClass->AddInternalCall("Internal_ConvexCastAll", (void*)&ScriptPhysicsScene::InternalConvexCastAll);
	metaData.ScriptClass->AddInternalCall("Internal_RayCastAny", (void*)&ScriptPhysicsScene::InternalRayCastAny);
	metaData.ScriptClass->AddInternalCall("Internal_RayCastAny0", (void*)&ScriptPhysicsScene::InternalRayCastAny0);
	metaData.ScriptClass->AddInternalCall("Internal_BoxCastAny", (void*)&ScriptPhysicsScene::InternalBoxCastAny);
	metaData.ScriptClass->AddInternalCall("Internal_SphereCastAny", (void*)&ScriptPhysicsScene::InternalSphereCastAny);
	metaData.ScriptClass->AddInternalCall("Internal_CapsuleCastAny", (void*)&ScriptPhysicsScene::InternalCapsuleCastAny);
	metaData.ScriptClass->AddInternalCall("Internal_ConvexCastAny", (void*)&ScriptPhysicsScene::InternalConvexCastAny);
	metaData.ScriptClass->AddInternalCall("Internal_BoxOverlap", (void*)&ScriptPhysicsScene::InternalBoxOverlap);
	metaData.ScriptClass->AddInternalCall("Internal_SphereOverlap", (void*)&ScriptPhysicsScene::InternalSphereOverlap);
	metaData.ScriptClass->AddInternalCall("Internal_CapsuleOverlap", (void*)&ScriptPhysicsScene::InternalCapsuleOverlap);
	metaData.ScriptClass->AddInternalCall("Internal_ConvexOverlap", (void*)&ScriptPhysicsScene::InternalConvexOverlap);
	metaData.ScriptClass->AddInternalCall("Internal_BoxOverlapAny", (void*)&ScriptPhysicsScene::InternalBoxOverlapAny);
	metaData.ScriptClass->AddInternalCall("Internal_SphereOverlapAny", (void*)&ScriptPhysicsScene::InternalSphereOverlapAny);
	metaData.ScriptClass->AddInternalCall("Internal_CapsuleOverlapAny", (void*)&ScriptPhysicsScene::InternalCapsuleOverlapAny);
	metaData.ScriptClass->AddInternalCall("Internal_ConvexOverlapAny", (void*)&ScriptPhysicsScene::InternalConvexOverlapAny);
	metaData.ScriptClass->AddInternalCall("Internal_GetGravity", (void*)&ScriptPhysicsScene::InternalGetGravity);
	metaData.ScriptClass->AddInternalCall("Internal_SetGravity", (void*)&ScriptPhysicsScene::InternalSetGravity);
	metaData.ScriptClass->AddInternalCall("Internal_AddBroadPhaseRegion", (void*)&ScriptPhysicsScene::InternalAddBroadPhaseRegion);
	metaData.ScriptClass->AddInternalCall("Internal_RemoveBroadPhaseRegion", (void*)&ScriptPhysicsScene::InternalRemoveBroadPhaseRegion);
	metaData.ScriptClass->AddInternalCall("Internal_ClearBroadPhaseRegions", (void*)&ScriptPhysicsScene::InternalClearBroadPhaseRegions);
}

MonoObject* ScriptPhysicsScene::Create(const SPtr<PhysicsScene>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptPhysicsScene>()) ScriptPhysicsScene(managedInstance, value);
	return managedInstance;
}

bool ScriptPhysicsScene::InternalRayCast(ScriptPhysicsScene* thisPtr, Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
{
	bool tmp__output;
	PhysicsQueryHit tmphit;
	tmp__output = thisPtr->GetInternal()->RayCast(*ray, tmphit, layer, max);

	bool __output;
	__output = tmp__output;
	__PhysicsQueryHitInterop interophit;
	interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
	MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

	return __output;
}

bool ScriptPhysicsScene::InternalRayCast0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
{
	bool tmp__output;
	PhysicsQueryHit tmphit;
	tmp__output = thisPtr->GetInternal()->RayCast(*origin, *unitDir, tmphit, layer, max);

	bool __output;
	__output = tmp__output;
	__PhysicsQueryHitInterop interophit;
	interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
	MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

	return __output;
}

bool ScriptPhysicsScene::InternalBoxCast(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
{
	bool tmp__output;
	PhysicsQueryHit tmphit;
	tmp__output = thisPtr->GetInternal()->BoxCast(*box, *rotation, *unitDir, tmphit, layer, max);

	bool __output;
	__output = tmp__output;
	__PhysicsQueryHitInterop interophit;
	interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
	MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

	return __output;
}

bool ScriptPhysicsScene::InternalSphereCast(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
{
	bool tmp__output;
	PhysicsQueryHit tmphit;
	tmp__output = thisPtr->GetInternal()->SphereCast(*sphere, *unitDir, tmphit, layer, max);

	bool __output;
	__output = tmp__output;
	__PhysicsQueryHitInterop interophit;
	interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
	MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

	return __output;
}

bool ScriptPhysicsScene::InternalCapsuleCast(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
{
	bool tmp__output;
	PhysicsQueryHit tmphit;
	tmp__output = thisPtr->GetInternal()->CapsuleCast(*capsule, *rotation, *unitDir, tmphit, layer, max);

	bool __output;
	__output = tmp__output;
	__PhysicsQueryHitInterop interophit;
	interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
	MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

	return __output;
}

bool ScriptPhysicsScene::InternalConvexCast(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
{
	bool tmp__output;
	ResourceHandle<PhysicsMesh> tmpmesh;
	ScriptRRefBase* scriptmesh;
	scriptmesh = ScriptRRefBase::ToNative(mesh);
	if(scriptmesh != nullptr)
		tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
	PhysicsQueryHit tmphit;
	tmp__output = thisPtr->GetInternal()->ConvexCast(tmpmesh, *position, *rotation, *unitDir, tmphit, layer, max);

	bool __output;
	__output = tmp__output;
	__PhysicsQueryHitInterop interophit;
	interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
	MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalRayCastAll(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max)
{
	Vector<PhysicsQueryHit> vec__output;
	vec__output = thisPtr->GetInternal()->RayCastAll(*ray, layer, max);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptPhysicsQueryHit>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptPhysicsQueryHit::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalRayCastAll0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
{
	Vector<PhysicsQueryHit> vec__output;
	vec__output = thisPtr->GetInternal()->RayCastAll(*origin, *unitDir, layer, max);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptPhysicsQueryHit>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptPhysicsQueryHit::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalBoxCastAll(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
{
	Vector<PhysicsQueryHit> vec__output;
	vec__output = thisPtr->GetInternal()->BoxCastAll(*box, *rotation, *unitDir, layer, max);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptPhysicsQueryHit>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptPhysicsQueryHit::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalSphereCastAll(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
{
	Vector<PhysicsQueryHit> vec__output;
	vec__output = thisPtr->GetInternal()->SphereCastAll(*sphere, *unitDir, layer, max);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptPhysicsQueryHit>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptPhysicsQueryHit::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalCapsuleCastAll(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
{
	Vector<PhysicsQueryHit> vec__output;
	vec__output = thisPtr->GetInternal()->CapsuleCastAll(*capsule, *rotation, *unitDir, layer, max);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptPhysicsQueryHit>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptPhysicsQueryHit::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalConvexCastAll(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
{
	Vector<PhysicsQueryHit> vec__output;
	ResourceHandle<PhysicsMesh> tmpmesh;
	ScriptRRefBase* scriptmesh;
	scriptmesh = ScriptRRefBase::ToNative(mesh);
	if(scriptmesh != nullptr)
		tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
	vec__output = thisPtr->GetInternal()->ConvexCastAll(tmpmesh, *position, *rotation, *unitDir, layer, max);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptPhysicsQueryHit>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptPhysicsQueryHit::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

bool ScriptPhysicsScene::InternalRayCastAny(ScriptPhysicsScene* thisPtr, Ray* ray, uint64_t layer, float max)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->RayCastAny(*ray, layer, max);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalRayCastAny0(ScriptPhysicsScene* thisPtr, Vector3* origin, Vector3* unitDir, uint64_t layer, float max)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->RayCastAny(*origin, *unitDir, layer, max);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalBoxCastAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->BoxCastAny(*box, *rotation, *unitDir, layer, max);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalSphereCastAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, Vector3* unitDir, uint64_t layer, float max)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->SphereCastAny(*sphere, *unitDir, layer, max);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalCapsuleCastAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->CapsuleCastAny(*capsule, *rotation, *unitDir, layer, max);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalConvexCastAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, Vector3* unitDir, uint64_t layer, float max)
{
	bool tmp__output;
	ResourceHandle<PhysicsMesh> tmpmesh;
	ScriptRRefBase* scriptmesh;
	scriptmesh = ScriptRRefBase::ToNative(mesh);
	if(scriptmesh != nullptr)
		tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
	tmp__output = thisPtr->GetInternal()->ConvexCastAny(tmpmesh, *position, *rotation, *unitDir, layer, max);

	bool __output;
	__output = tmp__output;

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalBoxOverlap(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer)
{
	Vector<GameObjectHandle<CCollider>> vec__output;
	vec__output = thisPtr->GetInternal()->BoxOverlap(*box, *rotation, layer);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptCCollider>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		ScriptComponentBase* script__output = nullptr;
		if(vec__output[i])
			script__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
		if(script__output != nullptr)
			array__output.Set(i, script__output->GetManagedInstance());
		else
			array__output.Set(i, nullptr);
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalSphereOverlap(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer)
{
	Vector<GameObjectHandle<CCollider>> vec__output;
	vec__output = thisPtr->GetInternal()->SphereOverlap(*sphere, layer);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptCCollider>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		ScriptComponentBase* script__output = nullptr;
		if(vec__output[i])
			script__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
		if(script__output != nullptr)
			array__output.Set(i, script__output->GetManagedInstance());
		else
			array__output.Set(i, nullptr);
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalCapsuleOverlap(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer)
{
	Vector<GameObjectHandle<CCollider>> vec__output;
	vec__output = thisPtr->GetInternal()->CapsuleOverlap(*capsule, *rotation, layer);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptCCollider>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		ScriptComponentBase* script__output = nullptr;
		if(vec__output[i])
			script__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
		if(script__output != nullptr)
			array__output.Set(i, script__output->GetManagedInstance());
		else
			array__output.Set(i, nullptr);
	}
	__output = array__output.GetInternal();

	return __output;
}

MonoArray* ScriptPhysicsScene::InternalConvexOverlap(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer)
{
	Vector<GameObjectHandle<CCollider>> vec__output;
	ResourceHandle<PhysicsMesh> tmpmesh;
	ScriptRRefBase* scriptmesh;
	scriptmesh = ScriptRRefBase::ToNative(mesh);
	if(scriptmesh != nullptr)
		tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
	vec__output = thisPtr->GetInternal()->ConvexOverlap(tmpmesh, *position, *rotation, layer);

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptCCollider>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		ScriptComponentBase* script__output = nullptr;
		if(vec__output[i])
			script__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(vec__output[i]));
		if(script__output != nullptr)
			array__output.Set(i, script__output->GetManagedInstance());
		else
			array__output.Set(i, nullptr);
	}
	__output = array__output.GetInternal();

	return __output;
}

bool ScriptPhysicsScene::InternalBoxOverlapAny(ScriptPhysicsScene* thisPtr, AABox* box, Quaternion* rotation, uint64_t layer)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->BoxOverlapAny(*box, *rotation, layer);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalSphereOverlapAny(ScriptPhysicsScene* thisPtr, Sphere* sphere, uint64_t layer)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->SphereOverlapAny(*sphere, layer);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalCapsuleOverlapAny(ScriptPhysicsScene* thisPtr, Capsule* capsule, Quaternion* rotation, uint64_t layer)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->CapsuleOverlapAny(*capsule, *rotation, layer);

	bool __output;
	__output = tmp__output;

	return __output;
}

bool ScriptPhysicsScene::InternalConvexOverlapAny(ScriptPhysicsScene* thisPtr, MonoObject* mesh, Vector3* position, Quaternion* rotation, uint64_t layer)
{
	bool tmp__output;
	ResourceHandle<PhysicsMesh> tmpmesh;
	ScriptRRefBase* scriptmesh;
	scriptmesh = ScriptRRefBase::ToNative(mesh);
	if(scriptmesh != nullptr)
		tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
	tmp__output = thisPtr->GetInternal()->ConvexOverlapAny(tmpmesh, *position, *rotation, layer);

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptPhysicsScene::InternalGetGravity(ScriptPhysicsScene* thisPtr, Vector3* __output)
{
	Vector3 tmp__output;
	tmp__output = thisPtr->GetInternal()->GetGravity();

	*__output = tmp__output;
}

void ScriptPhysicsScene::InternalSetGravity(ScriptPhysicsScene* thisPtr, Vector3* gravity)
{
	thisPtr->GetInternal()->SetGravity(*gravity);
}

uint32_t ScriptPhysicsScene::InternalAddBroadPhaseRegion(ScriptPhysicsScene* thisPtr, AABox* region)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->AddBroadPhaseRegion(*region);

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptPhysicsScene::InternalRemoveBroadPhaseRegion(ScriptPhysicsScene* thisPtr, uint32_t handle)
{
	thisPtr->GetInternal()->RemoveBroadPhaseRegion(handle);
}

void ScriptPhysicsScene::InternalClearBroadPhaseRegions(ScriptPhysicsScene* thisPtr)
{
	thisPtr->GetInternal()->ClearBroadPhaseRegions();
}
