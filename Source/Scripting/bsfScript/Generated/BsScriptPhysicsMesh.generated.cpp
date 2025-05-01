//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicsMesh.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"
#include "BsScriptRendererMeshData.generated.h"
#include "../Extensions/BsPhysicsMeshEx.h"

namespace bs
{
	ScriptPhysicsMesh::ScriptPhysicsMesh(const TResourceHandle<PhysicsMesh>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptPhysicsMesh::~ScriptPhysicsMesh()
	{
		UnregisterEvents();
	}

	void ScriptPhysicsMesh::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptPhysicsMesh::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptPhysicsMesh::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptPhysicsMesh::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMeshData", (void*)&ScriptPhysicsMesh::InternalGetMeshData);

	}

	MonoObject* ScriptPhysicsMesh::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptPhysicsMesh::InternalGetRef(ScriptPhysicsMesh* self)
	{
		return self->GetOrCreateResourceReference();
	}

	PhysicsMeshType ScriptPhysicsMesh::InternalGetType(ScriptPhysicsMesh* self)
	{
		PhysicsMeshType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PhysicsMesh*>(self->GetNativeObject())->GetType();

		PhysicsMeshType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMesh::InternalCreate(MonoObject* scriptObject, MonoObject* meshData, PhysicsMeshType type)
	{
		SPtr<RendererMeshData> tmpmeshData;
		ScriptRendererMeshData* scriptObjectWrappermeshData;
		scriptObjectWrappermeshData = ScriptRendererMeshData::GetScriptObjectWrapper(meshData);
		if(scriptObjectWrappermeshData != nullptr)
			tmpmeshData = std::static_pointer_cast<RendererMeshData>(scriptObjectWrappermeshData->GetBaseNativeObjectAsShared());
		TResourceHandle<PhysicsMesh> nativeObject = PhysicsMeshEx::Create(tmpmeshData, type);
		ScriptObjectWrapper::Create<ScriptPhysicsMesh>(nativeObject, scriptObject);
	}

	MonoObject* ScriptPhysicsMesh::InternalGetMeshData(ScriptPhysicsMesh* self)
	{
		SPtr<RendererMeshData> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = PhysicsMeshEx::GetMeshData(B3DStaticResourceCast<PhysicsMesh>(self->GetBaseNativeObjectAsHandle()));

		MonoObject* __output;
		__output = ScriptRendererMeshData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}
}
