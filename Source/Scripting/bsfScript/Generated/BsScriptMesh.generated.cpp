//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMesh.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptSkeleton.generated.h"
#include "BsScriptRendererMeshData.generated.h"
#include "BsScriptMorphShapes.generated.h"
#include "../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "../Extensions/BsMeshEx.h"
#include "BsScriptSubMesh.generated.h"

namespace bs
{
	ScriptMesh::ScriptMesh(MonoObject* managedInstance, const TResourceHandle<Mesh>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptMesh::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptMesh::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_GetSkeleton", (void*)&ScriptMesh::InternalGetSkeleton);
		metaData.ScriptClass->AddInternalCall("Internal_GetMorphShapes", (void*)&ScriptMesh::InternalGetMorphShapes);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMesh::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptMesh::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptMesh::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptMesh::InternalCreate2);
		metaData.ScriptClass->AddInternalCall("Internal_GetSubMeshes", (void*)&ScriptMesh::InternalGetSubMeshes);
		metaData.ScriptClass->AddInternalCall("Internal_GetSubMeshCount", (void*)&ScriptMesh::InternalGetSubMeshCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptMesh::InternalGetBounds);
		metaData.ScriptClass->AddInternalCall("Internal_GetMeshData", (void*)&ScriptMesh::InternalGetMeshData);
		metaData.ScriptClass->AddInternalCall("Internal_SetMeshData", (void*)&ScriptMesh::InternalSetMeshData);

	}

	 MonoObject*ScriptMesh::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptMesh::InternalGetRef(ScriptMesh* self)
	{
		return self->GetRRef();
	}

	MonoObject* ScriptMesh::InternalGetSkeleton(ScriptMesh* self)
	{
		SPtr<Skeleton> tmp__output;
		tmp__output = self->GetHandle()->GetSkeleton();

		MonoObject* __output;
		__output = ScriptSkeleton::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptMesh::InternalGetMorphShapes(ScriptMesh* self)
	{
		SPtr<MorphShapes> tmp__output;
		tmp__output = self->GetHandle()->GetMorphShapes();

		MonoObject* __output;
		__output = ScriptMorphShapes::Create(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalCreate(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(numVertices, numIndices, topology, usage, vertex, index);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}

	void ScriptMesh::InternalCreate0(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		Vector<SubMesh> nativeArraysubMeshes;
		if(subMeshes != nullptr)
		{
			ScriptArray scriptArraysubMeshes(subMeshes);
			nativeArraysubMeshes.resize(scriptArraysubMeshes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraysubMeshes.Size(); elementIndex++)
			{
				nativeArraysubMeshes[elementIndex] = scriptArraysubMeshes.Get<SubMesh>(elementIndex);
			}

		}
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(numVertices, numIndices, nativeArraysubMeshes, usage, vertex, index);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}

	void ScriptMesh::InternalCreate1(MonoObject* managedInstance, MonoObject* data, DrawOperationType topology, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptMeshData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptMeshData::ToNative(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = scriptObjectWrapperdata->GetInternal();
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(tmpdata, topology, usage);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}

	void ScriptMesh::InternalCreate2(MonoObject* managedInstance, MonoObject* data, MonoArray* subMeshes, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptMeshData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptMeshData::ToNative(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = scriptObjectWrapperdata->GetInternal();
		Vector<SubMesh> nativeArraysubMeshes;
		if(subMeshes != nullptr)
		{
			ScriptArray scriptArraysubMeshes(subMeshes);
			nativeArraysubMeshes.resize(scriptArraysubMeshes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraysubMeshes.Size(); elementIndex++)
			{
				nativeArraysubMeshes[elementIndex] = scriptArraysubMeshes.Get<SubMesh>(elementIndex);
			}

		}
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(tmpdata, nativeArraysubMeshes, usage);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}

	MonoArray* ScriptMesh::InternalGetSubMeshes(ScriptMesh* self)
	{
		Vector<SubMesh> nativeArray__output;
		nativeArray__output = MeshEx::GetSubMeshes(self->GetHandle());

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptSubMesh>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	uint32_t ScriptMesh::InternalGetSubMeshCount(ScriptMesh* self)
	{
		uint32_t tmp__output;
		tmp__output = MeshEx::GetSubMeshCount(self->GetHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMesh::InternalGetBounds(ScriptMesh* self, AABox* box, Sphere* sphere)
	{
		MeshEx::GetBounds(self->GetHandle(), box, sphere);
	}

	MonoObject* ScriptMesh::InternalGetMeshData(ScriptMesh* self)
	{
		SPtr<RendererMeshData> tmp__output;
		tmp__output = MeshEx::GetMeshData(self->GetHandle());

		MonoObject* __output;
		__output = ScriptMeshData::Create(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalSetMeshData(ScriptMesh* self, MonoObject* value)
	{
		SPtr<RendererMeshData> tmpvalue;
		ScriptMeshData* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptMeshData::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		MeshEx::SetMeshData(self->GetHandle(), tmpvalue);
	}
}
