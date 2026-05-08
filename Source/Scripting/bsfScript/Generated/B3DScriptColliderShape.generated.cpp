//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptColliderShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Engine/Core/Physics/B3DPhysicsMaterial.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptSphereColliderShapeInformation.generated.h"
#include "B3DScriptTQuaternion.generated.h"
#include "B3DScriptPlaneColliderShapeInformation.generated.h"
#include "B3DScriptBoxColliderShapeInformation.generated.h"
#include "B3DScriptCapsuleColliderShapeInformation.generated.h"
#include "B3DScriptMeshColliderShapeInformation.generated.h"
#include "B3DScriptColliderShape.generated.h"

namespace b3d
{
	ScriptColliderShape::ScriptColliderShape(const TShared<ColliderShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptColliderShape::~ScriptColliderShape()
	{
		UnregisterEvents();
	}

	void ScriptColliderShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptColliderShape::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptColliderShape::InternalSetPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptColliderShape::InternalGetPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRotation", (void*)&ScriptColliderShape::InternalSetRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptColliderShape::InternalGetRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScale", (void*)&ScriptColliderShape::InternalSetScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScale", (void*)&ScriptColliderShape::InternalGetScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsTrigger", (void*)&ScriptColliderShape::InternalSetIsTrigger);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIsTrigger", (void*)&ScriptColliderShape::InternalGetIsTrigger);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMass", (void*)&ScriptColliderShape::InternalSetMass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMass", (void*)&ScriptColliderShape::InternalGetMass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptColliderShape::InternalSetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptColliderShape::InternalGetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetContactOffset", (void*)&ScriptColliderShape::InternalSetContactOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetContactOffset", (void*)&ScriptColliderShape::InternalGetContactOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRestOffset", (void*)&ScriptColliderShape::InternalSetRestOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRestOffset", (void*)&ScriptColliderShape::InternalGetRestOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptColliderShape::InternalSetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptColliderShape::InternalGetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCollisionReportMode", (void*)&ScriptColliderShape::InternalSetCollisionReportMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCollisionReportMode", (void*)&ScriptColliderShape::InternalGetCollisionReportMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShape", (void*)&ScriptColliderShape::InternalSetShape);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShape0", (void*)&ScriptColliderShape::InternalSetShape0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShape1", (void*)&ScriptColliderShape::InternalSetShape1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShape2", (void*)&ScriptColliderShape::InternalSetShape2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShape3", (void*)&ScriptColliderShape::InternalSetShape3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPlaneShapeInformation", (void*)&ScriptColliderShape::InternalGetPlaneShapeInformation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBoxShapeInformation", (void*)&ScriptColliderShape::InternalGetBoxShapeInformation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSphereShapeInformation", (void*)&ScriptColliderShape::InternalGetSphereShapeInformation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCapsuleShapeInformation", (void*)&ScriptColliderShape::InternalGetCapsuleShapeInformation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMeshShapeInformation", (void*)&ScriptColliderShape::InternalGetMeshShapeInformation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreatePlane", (void*)&ScriptColliderShape::InternalCreatePlane);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateBox", (void*)&ScriptColliderShape::InternalCreateBox);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateSphere", (void*)&ScriptColliderShape::InternalCreateSphere);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateCapsule", (void*)&ScriptColliderShape::InternalCreateCapsule);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateMesh", (void*)&ScriptColliderShape::InternalCreateMesh);

	}

	MonoObject* ScriptColliderShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	ColliderShapeType ScriptColliderShape::InternalGetType(ScriptColliderShape* self)
	{
		ColliderShapeType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetType();

		ColliderShapeType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColliderShape::InternalSetPosition(ScriptColliderShape* self, TVector3<float>* position)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetPosition(*position);
	}

	void ScriptColliderShape::InternalGetPosition(ScriptColliderShape* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetPosition();

		*__output = tmp__output;
	}

	void ScriptColliderShape::InternalSetRotation(ScriptColliderShape* self, TQuaternion<float>* rotation)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetRotation(*rotation);
	}

	void ScriptColliderShape::InternalGetRotation(ScriptColliderShape* self, TQuaternion<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TQuaternion<float> tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetRotation();

		*__output = tmp__output;
	}

	void ScriptColliderShape::InternalSetScale(ScriptColliderShape* self, TVector3<float>* scale)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetScale(*scale);
	}

	void ScriptColliderShape::InternalGetScale(ScriptColliderShape* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetScale();

		*__output = tmp__output;
	}

	void ScriptColliderShape::InternalSetIsTrigger(ScriptColliderShape* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetIsTrigger(value);
	}

	bool ScriptColliderShape::InternalGetIsTrigger(ScriptColliderShape* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetIsTrigger();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColliderShape::InternalSetMass(ScriptColliderShape* self, float mass)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetMass(mass);
	}

	float ScriptColliderShape::InternalGetMass(ScriptColliderShape* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetMass();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColliderShape::InternalSetMaterial(ScriptColliderShape* self, MonoObject* material)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<PhysicsMaterial> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::GetScriptObjectWrapper(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<PhysicsMaterial>(scriptObjectWrappermaterial->GetNativeObject());
		static_cast<ColliderShape*>(self->GetNativeObject())->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptColliderShape::InternalGetMaterial(ScriptColliderShape* self)
	{
		TResourceHandle<PhysicsMaterial> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptColliderShape::InternalSetContactOffset(ScriptColliderShape* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetContactOffset(value);
	}

	float ScriptColliderShape::InternalGetContactOffset(ScriptColliderShape* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColliderShape::InternalSetRestOffset(ScriptColliderShape* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetRestOffset(value);
	}

	float ScriptColliderShape::InternalGetRestOffset(ScriptColliderShape* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetRestOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColliderShape::InternalSetLayer(ScriptColliderShape* self, uint64_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetLayer(layer);
	}

	uint64_t ScriptColliderShape::InternalGetLayer(ScriptColliderShape* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColliderShape::InternalSetCollisionReportMode(ScriptColliderShape* self, CollisionReportMode mode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetCollisionReportMode(mode);
	}

	CollisionReportMode ScriptColliderShape::InternalGetCollisionReportMode(ScriptColliderShape* self)
	{
		CollisionReportMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetCollisionReportMode();

		CollisionReportMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColliderShape::InternalSetShape(ScriptColliderShape* self, PlaneColliderShapeInformation* information)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetShape(*information);
	}

	void ScriptColliderShape::InternalSetShape0(ScriptColliderShape* self, __BoxColliderShapeInformationInterop* information)
	{
		if(!self->IsNativeObjectValid())
			return;

		BoxColliderShapeInformation tmpinformation;
		tmpinformation = ScriptBoxColliderShapeInformation::FromInterop(*information);
		static_cast<ColliderShape*>(self->GetNativeObject())->SetShape(tmpinformation);
	}

	void ScriptColliderShape::InternalSetShape1(ScriptColliderShape* self, SphereColliderShapeInformation* information)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetShape(*information);
	}

	void ScriptColliderShape::InternalSetShape2(ScriptColliderShape* self, CapsuleColliderShapeInformation* information)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColliderShape*>(self->GetNativeObject())->SetShape(*information);
	}

	void ScriptColliderShape::InternalSetShape3(ScriptColliderShape* self, __MeshColliderShapeInformationInterop* information)
	{
		if(!self->IsNativeObjectValid())
			return;

		MeshColliderShapeInformation tmpinformation;
		tmpinformation = ScriptMeshColliderShapeInformation::FromInterop(*information);
		static_cast<ColliderShape*>(self->GetNativeObject())->SetShape(tmpinformation);
	}

	void ScriptColliderShape::InternalGetPlaneShapeInformation(ScriptColliderShape* self, PlaneColliderShapeInformation* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		PlaneColliderShapeInformation tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetPlaneShapeInformation();

		*__output = tmp__output;
	}

	void ScriptColliderShape::InternalGetBoxShapeInformation(ScriptColliderShape* self, __BoxColliderShapeInformationInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		BoxColliderShapeInformation tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetBoxShapeInformation();

		__BoxColliderShapeInformationInterop interop__output;
		interop__output = ScriptBoxColliderShapeInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptBoxColliderShapeInformation::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptColliderShape::InternalGetSphereShapeInformation(ScriptColliderShape* self, SphereColliderShapeInformation* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		SphereColliderShapeInformation tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetSphereShapeInformation();

		*__output = tmp__output;
	}

	void ScriptColliderShape::InternalGetCapsuleShapeInformation(ScriptColliderShape* self, CapsuleColliderShapeInformation* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		CapsuleColliderShapeInformation tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetCapsuleShapeInformation();

		*__output = tmp__output;
	}

	void ScriptColliderShape::InternalGetMeshShapeInformation(ScriptColliderShape* self, __MeshColliderShapeInformationInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		MeshColliderShapeInformation tmp__output;
		tmp__output = static_cast<ColliderShape*>(self->GetNativeObject())->GetMeshShapeInformation();

		__MeshColliderShapeInformationInterop interop__output;
		interop__output = ScriptMeshColliderShapeInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptMeshColliderShapeInformation::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptColliderShape::InternalCreatePlane(MonoObject* scriptObject, PlaneColliderShapeInformation* information)
	{
		TShared<ColliderShape> nativeObject = ColliderShape::CreatePlane(*information);
		ScriptObjectWrapper::Create<ScriptColliderShape>(nativeObject, scriptObject);
	}

	void ScriptColliderShape::InternalCreateBox(MonoObject* scriptObject, __BoxColliderShapeInformationInterop* information)
	{
		BoxColliderShapeInformation tmpinformation;
		tmpinformation = ScriptBoxColliderShapeInformation::FromInterop(*information);
		TShared<ColliderShape> nativeObject = ColliderShape::CreateBox(tmpinformation);
		ScriptObjectWrapper::Create<ScriptColliderShape>(nativeObject, scriptObject);
	}

	void ScriptColliderShape::InternalCreateSphere(MonoObject* scriptObject, SphereColliderShapeInformation* information)
	{
		TShared<ColliderShape> nativeObject = ColliderShape::CreateSphere(*information);
		ScriptObjectWrapper::Create<ScriptColliderShape>(nativeObject, scriptObject);
	}

	void ScriptColliderShape::InternalCreateCapsule(MonoObject* scriptObject, CapsuleColliderShapeInformation* information)
	{
		TShared<ColliderShape> nativeObject = ColliderShape::CreateCapsule(*information);
		ScriptObjectWrapper::Create<ScriptColliderShape>(nativeObject, scriptObject);
	}

	void ScriptColliderShape::InternalCreateMesh(MonoObject* scriptObject, __MeshColliderShapeInformationInterop* information)
	{
		MeshColliderShapeInformation tmpinformation;
		tmpinformation = ScriptMeshColliderShapeInformation::FromInterop(*information);
		TShared<ColliderShape> nativeObject = ColliderShape::CreateMesh(tmpinformation);
		ScriptObjectWrapper::Create<ScriptColliderShape>(nativeObject, scriptObject);
	}
}
