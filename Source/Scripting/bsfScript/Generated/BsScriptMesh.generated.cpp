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
	ScriptMesh::ScriptMesh(MonoObject* managedInstance, const ResourceHandle<Mesh>& value)
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
	MonoObject* ScriptMesh::InternalGetRef(ScriptMesh* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	MonoObject* ScriptMesh::InternalGetSkeleton(ScriptMesh* thisPtr)
	{
		SPtr<Skeleton> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSkeleton();

		MonoObject* __output;
		__output = ScriptSkeleton::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptMesh::InternalGetMorphShapes(ScriptMesh* thisPtr)
	{
		SPtr<MorphShapes> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMorphShapes();

		MonoObject* __output;
		__output = ScriptMorphShapes::Create(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalCreate(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		ResourceHandle<Mesh> instance = MeshEx::Create(numVertices, numIndices, topology, usage, vertex, index);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMesh::InternalCreate0(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		Vector<SubMesh> vecsubMeshes;
		if(subMeshes != nullptr)
		{
			ScriptArray arraysubMeshes(subMeshes);
			vecsubMeshes.resize(arraysubMeshes.Size());
			for(int i = 0; i < (int)arraysubMeshes.Size(); i++)
			{
				vecsubMeshes[i] = arraysubMeshes.Get<SubMesh>(i);
			}

		}
		ResourceHandle<Mesh> instance = MeshEx::Create(numVertices, numIndices, vecsubMeshes, usage, vertex, index);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMesh::InternalCreate1(MonoObject* managedInstance, MonoObject* data, DrawOperationType topology, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptMeshData* scriptdata;
		scriptdata = ScriptMeshData::ToNative(data);
		if(scriptdata != nullptr)
			tmpdata = scriptdata->GetInternal();
		ResourceHandle<Mesh> instance = MeshEx::Create(tmpdata, topology, usage);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMesh::InternalCreate2(MonoObject* managedInstance, MonoObject* data, MonoArray* subMeshes, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptMeshData* scriptdata;
		scriptdata = ScriptMeshData::ToNative(data);
		if(scriptdata != nullptr)
			tmpdata = scriptdata->GetInternal();
		Vector<SubMesh> vecsubMeshes;
		if(subMeshes != nullptr)
		{
			ScriptArray arraysubMeshes(subMeshes);
			vecsubMeshes.resize(arraysubMeshes.Size());
			for(int i = 0; i < (int)arraysubMeshes.Size(); i++)
			{
				vecsubMeshes[i] = arraysubMeshes.Get<SubMesh>(i);
			}

		}
		ResourceHandle<Mesh> instance = MeshEx::Create(tmpdata, vecsubMeshes, usage);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	MonoArray* ScriptMesh::InternalGetSubMeshes(ScriptMesh* thisPtr)
	{
		Vector<SubMesh> vec__output;
		vec__output = MeshEx::GetSubMeshes(thisPtr->GetHandle());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptSubMesh>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	uint32_t ScriptMesh::InternalGetSubMeshCount(ScriptMesh* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = MeshEx::GetSubMeshCount(thisPtr->GetHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMesh::InternalGetBounds(ScriptMesh* thisPtr, AABox* box, Sphere* sphere)
	{
		MeshEx::GetBounds(thisPtr->GetHandle(), box, sphere);
	}

	MonoObject* ScriptMesh::InternalGetMeshData(ScriptMesh* thisPtr)
	{
		SPtr<RendererMeshData> tmp__output;
		tmp__output = MeshEx::GetMeshData(thisPtr->GetHandle());

		MonoObject* __output;
		__output = ScriptMeshData::Create(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalSetMeshData(ScriptMesh* thisPtr, MonoObject* value)
	{
		SPtr<RendererMeshData> tmpvalue;
		ScriptMeshData* scriptvalue;
		scriptvalue = ScriptMeshData::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		MeshEx::SetMeshData(thisPtr->GetHandle(), tmpvalue);
	}
}
