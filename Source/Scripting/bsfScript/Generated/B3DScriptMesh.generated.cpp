//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMesh.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Mesh/B3DMesh.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptSkeleton.generated.h"
#include "B3DScriptRendererMeshData.generated.h"
#include "B3DScriptMorphShapes.generated.h"
#include "../../../Engine/Core/Mesh/B3DMesh.h"
#include "../Extensions/B3DMeshEx.h"
#include "B3DScriptSubMesh.generated.h"
#include "B3DScriptTAABox.generated.h"
#include "B3DScriptTSphere.generated.h"

namespace b3d
{
	ScriptMesh::ScriptMesh(const TResourceHandle<Mesh>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMesh::~ScriptMesh()
	{
		UnregisterEvents();
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
		TShared<Skeleton> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Mesh*>(self->GetNativeObject())->GetSkeleton();

		MonoObject* __output;
		__output = ScriptSkeleton::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptMesh::InternalGetMorphShapes(ScriptMesh* self)
	{
		TShared<MorphShapes> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Mesh*>(self->GetNativeObject())->GetMorphShapes();

		MonoObject* __output;
		__output = ScriptMorphShapes::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalCreate(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshFlag flags, VertexLayout vertex, IndexType index)
	{
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(numVertices, numIndices, topology, flags, vertex, index);
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	void ScriptMesh::InternalCreate0(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshFlag flags, VertexLayout vertex, IndexType index)
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
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(numVertices, numIndices, nativeArraysubMeshes, flags, vertex, index);
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	void ScriptMesh::InternalCreate1(MonoObject* scriptObject, MonoObject* data, DrawOperationType topology, MeshFlag flags)
	{
		TShared<RendererMeshData> tmpdata;
		ScriptRendererMeshData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptRendererMeshData::GetScriptObjectWrapper(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = std::static_pointer_cast<RendererMeshData>(scriptObjectWrapperdata->GetBaseNativeObjectAsShared());
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(tmpdata, topology, flags);
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	void ScriptMesh::InternalCreate2(MonoObject* scriptObject, MonoObject* data, MonoArray* subMeshes, MeshFlag flags)
	{
		TShared<RendererMeshData> tmpdata;
		ScriptRendererMeshData* scriptObjectWrapperdata;
		scriptObjectWrapperdata = ScriptRendererMeshData::GetScriptObjectWrapper(data);
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
		TResourceHandle<Mesh> nativeObject = MeshEx::Create(tmpdata, nativeArraysubMeshes, flags);
		ScriptObjectWrapper::Create<ScriptMesh>(nativeObject, scriptObject);
	}

	MonoArray* ScriptMesh::InternalGetSubMeshes(ScriptMesh* self)
	{
		Vector<SubMesh> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

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
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = MeshEx::GetSubMeshCount(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMesh::InternalGetBounds(ScriptMesh* self, __TAABox_float_Interop* box, __TSphere_float_Interop* sphere)
	{
		if(!self->IsNativeObjectValid())
			return;

		TAABox<float> tmpbox;
		TSphere<float> tmpsphere;
		MeshEx::GetBounds(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()), &tmpbox, &tmpsphere);

		__TAABox_float_Interop interopbox;
		interopbox = ScriptAABox::ToInterop(tmpbox);
		MonoUtil::ValueCopy(box, &interopbox, ScriptAABox::GetMetaData()->ScriptClass->GetInternalClass());
		__TSphere_float_Interop interopsphere;
		interopsphere = ScriptSphere::ToInterop(tmpsphere);
		MonoUtil::ValueCopy(sphere, &interopsphere, ScriptSphere::GetMetaData()->ScriptClass->GetInternalClass());
	}

	MonoObject* ScriptMesh::InternalGetMeshData(ScriptMesh* self)
	{
		TShared<RendererMeshData> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = MeshEx::GetMeshData(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()));

		MonoObject* __output;
		__output = ScriptRendererMeshData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptMesh::InternalSetMeshData(ScriptMesh* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<RendererMeshData> tmpvalue;
		ScriptRendererMeshData* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRendererMeshData::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<RendererMeshData>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		MeshEx::SetMeshData(B3DStaticResourceCast<Mesh>(self->GetBaseNativeObjectAsHandle()), tmpvalue);
	}
}
