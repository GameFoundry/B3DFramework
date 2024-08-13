//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicsScene.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysics.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptPhysicsQueryHit.generated.h"
#include "Wrappers/BsScriptQuaternion.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"
#include "BsScriptCCollider.generated.h"

namespace bs
{
	ScriptPhysicsScene::ScriptPhysicsScene(MonoObject* managedInstance, const SPtr<PhysicsScene>& value)
		:ScriptObject(managedInstance), mInternal(value)
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
		new (B3DAllocate<ScriptPhysicsScene>()) ScriptPhysicsScene(managedInstance, value);
		return managedInstance;
	}
	bool ScriptPhysicsScene::InternalRayCast(ScriptPhysicsScene* self, Ray* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = self->GetInternal()->RayCast(*ray, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

		return __output;
	}

	bool ScriptPhysicsScene::InternalRayCast0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = self->GetInternal()->RayCast(*origin, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

		return __output;
	}

	bool ScriptPhysicsScene::InternalBoxCast(ScriptPhysicsScene* self, AABox* box, Quaternion* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = self->GetInternal()->BoxCast(*box, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

		return __output;
	}

	bool ScriptPhysicsScene::InternalSphereCast(ScriptPhysicsScene* self, Sphere* sphere, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = self->GetInternal()->SphereCast(*sphere, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

		return __output;
	}

	bool ScriptPhysicsScene::InternalCapsuleCast(ScriptPhysicsScene* self, Capsule* capsule, Quaternion* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		PhysicsQueryHit tmphit;
		tmp__output = self->GetInternal()->CapsuleCast(*capsule, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

		return __output;
	}

	bool ScriptPhysicsScene::InternalConvexCast(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::ToNative(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetHandle());
		PhysicsQueryHit tmphit;
		tmp__output = self->GetInternal()->ConvexCast(tmpmesh, *position, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClassInternal());

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalRayCastAll(ScriptPhysicsScene* self, Ray* ray, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		nativeArray__output = self->GetInternal()->RayCastAll(*ray, layer, max);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPhysicsQueryHit>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptPhysicsQueryHit::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalRayCastAll0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		nativeArray__output = self->GetInternal()->RayCastAll(*origin, *unitDir, layer, max);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPhysicsQueryHit>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptPhysicsQueryHit::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalBoxCastAll(ScriptPhysicsScene* self, AABox* box, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		nativeArray__output = self->GetInternal()->BoxCastAll(*box, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPhysicsQueryHit>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptPhysicsQueryHit::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalSphereCastAll(ScriptPhysicsScene* self, Sphere* sphere, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		nativeArray__output = self->GetInternal()->SphereCastAll(*sphere, *unitDir, layer, max);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPhysicsQueryHit>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptPhysicsQueryHit::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalCapsuleCastAll(ScriptPhysicsScene* self, Capsule* capsule, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		nativeArray__output = self->GetInternal()->CapsuleCastAll(*capsule, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPhysicsQueryHit>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptPhysicsQueryHit::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalConvexCastAll(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::ToNative(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetHandle());
		nativeArray__output = self->GetInternal()->ConvexCastAll(tmpmesh, *position, *rotation, *unitDir, layer, max);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPhysicsQueryHit>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptPhysicsQueryHit::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	bool ScriptPhysicsScene::InternalRayCastAny(ScriptPhysicsScene* self, Ray* ray, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->RayCastAny(*ray, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalRayCastAny0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->RayCastAny(*origin, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalBoxCastAny(ScriptPhysicsScene* self, AABox* box, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->BoxCastAny(*box, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalSphereCastAny(ScriptPhysicsScene* self, Sphere* sphere, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->SphereCastAny(*sphere, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalCapsuleCastAny(ScriptPhysicsScene* self, Capsule* capsule, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->CapsuleCastAny(*capsule, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalConvexCastAny(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::ToNative(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetHandle());
		tmp__output = self->GetInternal()->ConvexCastAny(tmpmesh, *position, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalBoxOverlap(ScriptPhysicsScene* self, AABox* box, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> nativeArray__output;
		nativeArray__output = self->GetInternal()->BoxOverlap(*box, *rotation, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptComponentBase* scriptObjectWrapper__output = nullptr;
			if(nativeArray__output[elementIndex])
				scriptObjectWrapper__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(B3DStaticGameObjectCast<Component>(nativeArray__output[elementIndex]));
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetManagedInstance());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalSphereOverlap(ScriptPhysicsScene* self, Sphere* sphere, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> nativeArray__output;
		nativeArray__output = self->GetInternal()->SphereOverlap(*sphere, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptComponentBase* scriptObjectWrapper__output = nullptr;
			if(nativeArray__output[elementIndex])
				scriptObjectWrapper__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(B3DStaticGameObjectCast<Component>(nativeArray__output[elementIndex]));
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetManagedInstance());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalCapsuleOverlap(ScriptPhysicsScene* self, Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> nativeArray__output;
		nativeArray__output = self->GetInternal()->CapsuleOverlap(*capsule, *rotation, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptComponentBase* scriptObjectWrapper__output = nullptr;
			if(nativeArray__output[elementIndex])
				scriptObjectWrapper__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(B3DStaticGameObjectCast<Component>(nativeArray__output[elementIndex]));
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetManagedInstance());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalConvexOverlap(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, uint64_t layer)
	{
		Vector<GameObjectHandle<CCollider>> nativeArray__output;
		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::ToNative(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetHandle());
		nativeArray__output = self->GetInternal()->ConvexOverlap(tmpmesh, *position, *rotation, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptComponentBase* scriptObjectWrapper__output = nullptr;
			if(nativeArray__output[elementIndex])
				scriptObjectWrapper__output = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(B3DStaticGameObjectCast<Component>(nativeArray__output[elementIndex]));
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetManagedInstance());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	bool ScriptPhysicsScene::InternalBoxOverlapAny(ScriptPhysicsScene* self, AABox* box, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->BoxOverlapAny(*box, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalSphereOverlapAny(ScriptPhysicsScene* self, Sphere* sphere, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->SphereOverlapAny(*sphere, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalCapsuleOverlapAny(ScriptPhysicsScene* self, Capsule* capsule, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->CapsuleOverlapAny(*capsule, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalConvexOverlapAny(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, Quaternion* rotation, uint64_t layer)
	{
		bool tmp__output;
		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::ToNative(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetHandle());
		tmp__output = self->GetInternal()->ConvexOverlapAny(tmpmesh, *position, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::InternalGetGravity(ScriptPhysicsScene* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->GetGravity();

		*__output = tmp__output;
	}

	void ScriptPhysicsScene::InternalSetGravity(ScriptPhysicsScene* self, TVector3<float>* gravity)
	{
		self->GetInternal()->SetGravity(*gravity);
	}

	uint32_t ScriptPhysicsScene::InternalAddBroadPhaseRegion(ScriptPhysicsScene* self, AABox* region)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->AddBroadPhaseRegion(*region);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::InternalRemoveBroadPhaseRegion(ScriptPhysicsScene* self, uint32_t handle)
	{
		self->GetInternal()->RemoveBroadPhaseRegion(handle);
	}

	void ScriptPhysicsScene::InternalClearBroadPhaseRegions(ScriptPhysicsScene* self)
	{
		self->GetInternal()->ClearBroadPhaseRegions();
	}
}
