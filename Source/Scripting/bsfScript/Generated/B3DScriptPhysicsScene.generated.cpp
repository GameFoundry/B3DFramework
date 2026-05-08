//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPhysicsScene.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Physics/B3DPhysics.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Engine/Core/Physics/B3DPhysicsMesh.h"
#include "B3DScriptTRay.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptPhysicsQueryHit.generated.h"
#include "B3DScriptTAABox.generated.h"
#include "B3DScriptTQuaternion.generated.h"
#include "B3DScriptTSphere.generated.h"
#include "../../../Engine/Core/Components/B3DCollider.h"
#include "B3DScriptCollider.generated.h"

namespace b3d
{
	ScriptPhysicsScene::ScriptPhysicsScene(const TShared<PhysicsScene>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptPhysicsScene::~ScriptPhysicsScene()
	{
		UnregisterEvents();
	}

	void ScriptPhysicsScene::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RayCast", (void*)&ScriptPhysicsScene::InternalRayCast);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RayCast0", (void*)&ScriptPhysicsScene::InternalRayCast0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_BoxCast", (void*)&ScriptPhysicsScene::InternalBoxCast);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SphereCast", (void*)&ScriptPhysicsScene::InternalSphereCast);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CapsuleCast", (void*)&ScriptPhysicsScene::InternalCapsuleCast);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ConvexCast", (void*)&ScriptPhysicsScene::InternalConvexCast);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RayCastAll", (void*)&ScriptPhysicsScene::InternalRayCastAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RayCastAll0", (void*)&ScriptPhysicsScene::InternalRayCastAll0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_BoxCastAll", (void*)&ScriptPhysicsScene::InternalBoxCastAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SphereCastAll", (void*)&ScriptPhysicsScene::InternalSphereCastAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CapsuleCastAll", (void*)&ScriptPhysicsScene::InternalCapsuleCastAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ConvexCastAll", (void*)&ScriptPhysicsScene::InternalConvexCastAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RayCastAny", (void*)&ScriptPhysicsScene::InternalRayCastAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RayCastAny0", (void*)&ScriptPhysicsScene::InternalRayCastAny0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_BoxCastAny", (void*)&ScriptPhysicsScene::InternalBoxCastAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SphereCastAny", (void*)&ScriptPhysicsScene::InternalSphereCastAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CapsuleCastAny", (void*)&ScriptPhysicsScene::InternalCapsuleCastAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ConvexCastAny", (void*)&ScriptPhysicsScene::InternalConvexCastAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_BoxOverlap", (void*)&ScriptPhysicsScene::InternalBoxOverlap);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SphereOverlap", (void*)&ScriptPhysicsScene::InternalSphereOverlap);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CapsuleOverlap", (void*)&ScriptPhysicsScene::InternalCapsuleOverlap);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ConvexOverlap", (void*)&ScriptPhysicsScene::InternalConvexOverlap);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_BoxOverlapAny", (void*)&ScriptPhysicsScene::InternalBoxOverlapAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SphereOverlapAny", (void*)&ScriptPhysicsScene::InternalSphereOverlapAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CapsuleOverlapAny", (void*)&ScriptPhysicsScene::InternalCapsuleOverlapAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ConvexOverlapAny", (void*)&ScriptPhysicsScene::InternalConvexOverlapAny);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGravity", (void*)&ScriptPhysicsScene::InternalGetGravity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGravity", (void*)&ScriptPhysicsScene::InternalSetGravity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddBroadPhaseRegion", (void*)&ScriptPhysicsScene::InternalAddBroadPhaseRegion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveBroadPhaseRegion", (void*)&ScriptPhysicsScene::InternalRemoveBroadPhaseRegion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClearBroadPhaseRegions", (void*)&ScriptPhysicsScene::InternalClearBroadPhaseRegions);

	}

	MonoObject* ScriptPhysicsScene::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	bool ScriptPhysicsScene::InternalRayCast(ScriptPhysicsScene* self, __TRay_float_Interop* ray, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TRay<float> tmpray;
		tmpray = ScriptRay::FromInterop(*ray);
		PhysicsQueryHit tmphit;
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->RayCast(tmpray, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::InternalRayCast0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		PhysicsQueryHit tmphit;
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->RayCast(*origin, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::InternalBoxCast(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TAABox<float> tmpbox;
		tmpbox = ScriptAABox::FromInterop(*box);
		PhysicsQueryHit tmphit;
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->BoxCast(tmpbox, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::InternalSphereCast(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TSphere<float> tmpsphere;
		tmpsphere = ScriptSphere::FromInterop(*sphere);
		PhysicsQueryHit tmphit;
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->SphereCast(tmpsphere, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::InternalCapsuleCast(ScriptPhysicsScene* self, TCapsule<float>* capsule, TQuaternion<float>* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		PhysicsQueryHit tmphit;
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->CapsuleCast(*capsule, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClass());

		return __output;
	}

	bool ScriptPhysicsScene::InternalConvexCast(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, TVector3<float>* unitDir, __PhysicsQueryHitInterop* hit, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::GetScriptObjectWrapper(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetNativeObject());
		PhysicsQueryHit tmphit;
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->ConvexCast(tmpmesh, *position, *rotation, *unitDir, tmphit, layer, max);

		bool __output;
		__output = tmp__output;
		__PhysicsQueryHitInterop interophit;
		interophit = ScriptPhysicsQueryHit::ToInterop(tmphit);
		MonoUtil::ValueCopy(hit, &interophit, ScriptPhysicsQueryHit::GetMetaData()->ScriptClass->GetInternalClass());

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalRayCastAll(ScriptPhysicsScene* self, __TRay_float_Interop* ray, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		TRay<float> tmpray;
		tmpray = ScriptRay::FromInterop(*ray);
		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->RayCastAll(tmpray, layer, max);

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
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->RayCastAll(*origin, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::InternalBoxCastAll(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		TAABox<float> tmpbox;
		tmpbox = ScriptAABox::FromInterop(*box);
		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->BoxCastAll(tmpbox, *rotation, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::InternalSphereCastAll(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		TSphere<float> tmpsphere;
		tmpsphere = ScriptSphere::FromInterop(*sphere);
		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->SphereCastAll(tmpsphere, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::InternalCapsuleCastAll(ScriptPhysicsScene* self, TCapsule<float>* capsule, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->CapsuleCastAll(*capsule, *rotation, *unitDir, layer, max);

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

	MonoArray* ScriptPhysicsScene::InternalConvexCastAll(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		Vector<PhysicsQueryHit> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::GetScriptObjectWrapper(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetNativeObject());
		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->ConvexCastAll(tmpmesh, *position, *rotation, *unitDir, layer, max);

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

	bool ScriptPhysicsScene::InternalRayCastAny(ScriptPhysicsScene* self, __TRay_float_Interop* ray, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TRay<float> tmpray;
		tmpray = ScriptRay::FromInterop(*ray);
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->RayCastAny(tmpray, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalRayCastAny0(ScriptPhysicsScene* self, TVector3<float>* origin, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->RayCastAny(*origin, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalBoxCastAny(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TAABox<float> tmpbox;
		tmpbox = ScriptAABox::FromInterop(*box);
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->BoxCastAny(tmpbox, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalSphereCastAny(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TSphere<float> tmpsphere;
		tmpsphere = ScriptSphere::FromInterop(*sphere);
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->SphereCastAny(tmpsphere, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalCapsuleCastAny(ScriptPhysicsScene* self, TCapsule<float>* capsule, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->CapsuleCastAny(*capsule, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalConvexCastAny(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, TVector3<float>* unitDir, uint64_t layer, float max)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::GetScriptObjectWrapper(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetNativeObject());
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->ConvexCastAny(tmpmesh, *position, *rotation, *unitDir, layer, max);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalBoxOverlap(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, uint64_t layer)
	{
		Vector<TGameObjectHandle<Collider>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		TAABox<float> tmpbox;
		tmpbox = ScriptAABox::FromInterop(*box);
		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->BoxOverlap(tmpbox, *rotation, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			MonoObject* tempscriptArray__output = nullptr;
			if(nativeArray__output[elementIndex])
				tempscriptArray__output = ScriptComponent::GetOrCreateScriptObject(nativeArray__output[elementIndex]);
			scriptArray__output.Set(elementIndex, tempscriptArray__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalSphereOverlap(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, uint64_t layer)
	{
		Vector<TGameObjectHandle<Collider>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		TSphere<float> tmpsphere;
		tmpsphere = ScriptSphere::FromInterop(*sphere);
		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->SphereOverlap(tmpsphere, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			MonoObject* tempscriptArray__output = nullptr;
			if(nativeArray__output[elementIndex])
				tempscriptArray__output = ScriptComponent::GetOrCreateScriptObject(nativeArray__output[elementIndex]);
			scriptArray__output.Set(elementIndex, tempscriptArray__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalCapsuleOverlap(ScriptPhysicsScene* self, TCapsule<float>* capsule, TQuaternion<float>* rotation, uint64_t layer)
	{
		Vector<TGameObjectHandle<Collider>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->CapsuleOverlap(*capsule, *rotation, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			MonoObject* tempscriptArray__output = nullptr;
			if(nativeArray__output[elementIndex])
				tempscriptArray__output = ScriptComponent::GetOrCreateScriptObject(nativeArray__output[elementIndex]);
			scriptArray__output.Set(elementIndex, tempscriptArray__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptPhysicsScene::InternalConvexOverlap(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, uint64_t layer)
	{
		Vector<TGameObjectHandle<Collider>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::GetScriptObjectWrapper(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetNativeObject());
		nativeArray__output = static_cast<PhysicsScene*>(self->GetNativeObject())->ConvexOverlap(tmpmesh, *position, *rotation, layer);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCollider>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			MonoObject* tempscriptArray__output = nullptr;
			if(nativeArray__output[elementIndex])
				tempscriptArray__output = ScriptComponent::GetOrCreateScriptObject(nativeArray__output[elementIndex]);
			scriptArray__output.Set(elementIndex, tempscriptArray__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	bool ScriptPhysicsScene::InternalBoxOverlapAny(ScriptPhysicsScene* self, __TAABox_float_Interop* box, TQuaternion<float>* rotation, uint64_t layer)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TAABox<float> tmpbox;
		tmpbox = ScriptAABox::FromInterop(*box);
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->BoxOverlapAny(tmpbox, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalSphereOverlapAny(ScriptPhysicsScene* self, __TSphere_float_Interop* sphere, uint64_t layer)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TSphere<float> tmpsphere;
		tmpsphere = ScriptSphere::FromInterop(*sphere);
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->SphereOverlapAny(tmpsphere, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalCapsuleOverlapAny(ScriptPhysicsScene* self, TCapsule<float>* capsule, TQuaternion<float>* rotation, uint64_t layer)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->CapsuleOverlapAny(*capsule, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysicsScene::InternalConvexOverlapAny(ScriptPhysicsScene* self, MonoObject* mesh, TVector3<float>* position, TQuaternion<float>* rotation, uint64_t layer)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::GetScriptObjectWrapper(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetNativeObject());
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->ConvexOverlapAny(tmpmesh, *position, *rotation, layer);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::InternalGetGravity(ScriptPhysicsScene* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->GetGravity();

		*__output = tmp__output;
	}

	void ScriptPhysicsScene::InternalSetGravity(ScriptPhysicsScene* self, TVector3<float>* gravity)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PhysicsScene*>(self->GetNativeObject())->SetGravity(*gravity);
	}

	uint32_t ScriptPhysicsScene::InternalAddBroadPhaseRegion(ScriptPhysicsScene* self, __TAABox_float_Interop* region)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TAABox<float> tmpregion;
		tmpregion = ScriptAABox::FromInterop(*region);
		tmp__output = static_cast<PhysicsScene*>(self->GetNativeObject())->AddBroadPhaseRegion(tmpregion);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsScene::InternalRemoveBroadPhaseRegion(ScriptPhysicsScene* self, uint32_t handle)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PhysicsScene*>(self->GetNativeObject())->RemoveBroadPhaseRegion(handle);
	}

	void ScriptPhysicsScene::InternalClearBroadPhaseRegions(ScriptPhysicsScene* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PhysicsScene*>(self->GetNativeObject())->ClearBroadPhaseRegions();
	}
}
