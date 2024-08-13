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
	ScriptPhysicsMesh::ScriptPhysicsMesh(MonoObject* managedInstance, const TResourceHandle<PhysicsMesh>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptPhysicsMesh::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptPhysicsMesh::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptPhysicsMesh::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptPhysicsMesh::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_GetMeshData", (void*)&ScriptPhysicsMesh::InternalGetMeshData);

	}

	 MonoObject*ScriptPhysicsMesh::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptPhysicsMesh::InternalGetRef(ScriptPhysicsMesh* self)
	{
		return self->GetRRef();
	}

	PhysicsMeshType ScriptPhysicsMesh::InternalGetType(ScriptPhysicsMesh* self)
	{
		PhysicsMeshType tmp__output;
		tmp__output = self->GetHandle()->GetType();

		PhysicsMeshType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMesh::InternalCreate(MonoObject* managedInstance, MonoObject* meshData, PhysicsMeshType type)
	{
		SPtr<RendererMeshData> tmpmeshData;
		ScriptMeshData* scriptObjectWrappermeshData;
		scriptObjectWrappermeshData = ScriptMeshData::ToNative(meshData);
		if(scriptObjectWrappermeshData != nullptr)
			tmpmeshData = scriptObjectWrappermeshData->GetInternal();
		TResourceHandle<PhysicsMesh> nativeObject = PhysicsMeshEx::Create(tmpmeshData, type);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}

	MonoObject* ScriptPhysicsMesh::InternalGetMeshData(ScriptPhysicsMesh* self)
	{
		SPtr<RendererMeshData> tmp__output;
		tmp__output = PhysicsMeshEx::GetMeshData(self->GetHandle());

		MonoObject* __output;
		__output = ScriptMeshData::Create(tmp__output);

		return __output;
	}
}
