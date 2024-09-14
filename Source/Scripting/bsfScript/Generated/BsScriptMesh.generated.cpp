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
	ScriptMesh::ScriptMesh(const TResourceHandle<Mesh>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptMesh::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptMesh::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSkeleton", (void*)&ScriptMesh::InternalGetSkeleton);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMorphShapes", (void*)&ScriptMesh::InternalGetMorphShapes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMesh::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptMesh::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptMesh::InternalCreate1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptMesh::InternalCreate2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSubMeshes", (void*)&ScriptMesh::InternalGetSubMeshes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSubMeshCount", (void*)&ScriptMesh::InternalGetSubMeshCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptMesh::InternalGetBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMeshData", (void*)&ScriptMesh::InternalGetMeshData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMeshData", (void*)&ScriptMesh::InternalSetMeshData);

	}

	MonoObject* ScriptMesh::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptMesh::InternalGetRef(ScriptMesh* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoObject* ScriptMesh::InternalGetSkeleton(ScriptMesh* self)
	{
		SPtr<Skeleton> tmp__output;
		tmp__output = static_cast<Mesh*>(self->GetNativeObject())->GetSkeleton();

		MonoObject* __output;
		__output = ScriptSkeleton::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptMesh::InternalGetMorphShapes(ScriptMesh* self)
	{
		SPtr<MorphShapes> tmp__output;
		tmp__output = static_cast<Mesh*>(self->GetNativeObject())->GetMorphShapes();

		MonoObject* __output;
		__output = ScriptMorphShapes::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalCreate(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(numVertices, numIndices, topology, usage, vertex, index);
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	void ScriptMesh::InternalCreate0(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index)
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
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	void ScriptMesh::InternalCreate1(MonoObject* scriptObject, MonoObject* data, DrawOperationType topology, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptMeshData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptMeshData::GetScriptObjectWrapper(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = std::static_pointer_cast<RendererMeshData>(scriptObjectWrapperdata->GetBaseNativeObjectAsShared());
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(tmpdata, topology, usage);
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	void ScriptMesh::InternalCreate2(MonoObject* scriptObject, MonoObject* data, MonoArray* subMeshes, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptMeshData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptMeshData::GetScriptObjectWrapper(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = std::static_pointer_cast<RendererMeshData>(scriptObjectWrapperdata->GetBaseNativeObjectAsShared());
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
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	MonoArray* ScriptMesh::InternalGetSubMeshes(ScriptMesh* self)
	{
		Vector<SubMesh> nativeArray__output;
		nativeArray__output = MeshEx::GetSubMeshes(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()));

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
		tmp__output = MeshEx::GetSubMeshCount(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMesh::InternalGetBounds(ScriptMesh* self, AABox* box, Sphere* sphere)
	{
		MeshEx::GetBounds(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()), box, sphere);
	}

	MonoObject* ScriptMesh::InternalGetMeshData(ScriptMesh* self)
	{
		SPtr<RendererMeshData> tmp__output;
		tmp__output = MeshEx::GetMeshData(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()));

		MonoObject* __output;
		__output = ScriptMeshData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalSetMeshData(ScriptMesh* self, MonoObject* value)
	{
		SPtr<RendererMeshData> tmpvalue;
		ScriptMeshData* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptMeshData::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<RendererMeshData>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		MeshEx::SetMeshData(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()), tmpvalue);
	}
}
