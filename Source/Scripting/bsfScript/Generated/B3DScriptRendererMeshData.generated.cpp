//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRendererMeshData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Renderer/B3DRendererMeshData.h"
#include "B3DScriptTVector4.generated.h"
#include "B3DScriptMeshData.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptRendererMeshData.generated.h"
#include "../Extensions/B3DMeshDataEx.h"
#include "B3DScriptColor.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptBoneWeight.generated.h"

namespace b3d
{
	ScriptRendererMeshData::ScriptRendererMeshData(const TShared<RendererMeshData>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRendererMeshData::~ScriptRendererMeshData()
	{
		UnregisterEvents();
	}

	void ScriptRendererMeshData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetData", (void*)&ScriptRendererMeshData::InternalGetData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptRendererMeshData::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPositions", (void*)&ScriptRendererMeshData::InternalGetPositions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPositions", (void*)&ScriptRendererMeshData::InternalSetPositions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNormals", (void*)&ScriptRendererMeshData::InternalGetNormals);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNormals", (void*)&ScriptRendererMeshData::InternalSetNormals);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTangents", (void*)&ScriptRendererMeshData::InternalGetTangents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTangents", (void*)&ScriptRendererMeshData::InternalSetTangents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColors", (void*)&ScriptRendererMeshData::InternalGetColors);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColors", (void*)&ScriptRendererMeshData::InternalSetColors);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUV0", (void*)&ScriptRendererMeshData::InternalGetUV0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUV0", (void*)&ScriptRendererMeshData::InternalSetUV0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUV1", (void*)&ScriptRendererMeshData::InternalGetUV1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUV1", (void*)&ScriptRendererMeshData::InternalSetUV1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBoneWeights", (void*)&ScriptRendererMeshData::InternalGetBoneWeights);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBoneWeights", (void*)&ScriptRendererMeshData::InternalSetBoneWeights);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIndices", (void*)&ScriptRendererMeshData::InternalGetIndices);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIndices", (void*)&ScriptRendererMeshData::InternalSetIndices);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVertexCount", (void*)&ScriptRendererMeshData::InternalGetVertexCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIndexCount", (void*)&ScriptRendererMeshData::InternalGetIndexCount);

	}

	MonoObject* ScriptRendererMeshData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptRendererMeshData::InternalGetData(ScriptRendererMeshData* self)
	{
		TShared<MeshData> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RendererMeshData*>(self->GetNativeObject())->GetData();

		MonoObject* __output;
		__output = ScriptMeshData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRendererMeshData::InternalCreate(MonoObject* scriptObject, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType)
	{
		TShared<RendererMeshData> nativeObject = MeshDataEx::Create(numVertices, numIndices, layout, indexType);
		ScriptObjectWrapper::Create<ScriptRendererMeshData>(nativeObject, scriptObject);
	}

	MonoArray* ScriptRendererMeshData::InternalGetPositions(ScriptRendererMeshData* self)
	{
		Vector<TVector3<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetPositions(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptVector3>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetPositions(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector3<float>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<TVector3<float>>(elementIndex);
			}
		}
		MeshDataEx::SetPositions(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetNormals(ScriptRendererMeshData* self)
	{
		Vector<TVector3<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetNormals(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptVector3>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetNormals(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector3<float>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<TVector3<float>>(elementIndex);
			}
		}
		MeshDataEx::SetNormals(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetTangents(ScriptRendererMeshData* self)
	{
		Vector<TVector4<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetTangents(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptVector4>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetTangents(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector4<float>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<TVector4<float>>(elementIndex);
			}
		}
		MeshDataEx::SetTangents(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetColors(ScriptRendererMeshData* self)
	{
		Vector<Color> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetColors(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptColor>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetColors(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<Color> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<Color>(elementIndex);
			}
		}
		MeshDataEx::SetColors(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetUV0(ScriptRendererMeshData* self)
	{
		Vector<TVector2<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetUV0(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptTVector2_float_>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetUV0(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector2<float>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<TVector2<float>>(elementIndex);
			}
		}
		MeshDataEx::SetUV0(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetUV1(ScriptRendererMeshData* self)
	{
		Vector<TVector2<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetUV1(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptTVector2_float_>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetUV1(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector2<float>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<TVector2<float>>(elementIndex);
			}
		}
		MeshDataEx::SetUV1(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetBoneWeights(ScriptRendererMeshData* self)
	{
		Vector<BoneWeight> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetBoneWeights(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptBoneWeight>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetBoneWeights(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<BoneWeight> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<BoneWeight>(elementIndex);
			}
		}
		MeshDataEx::SetBoneWeights(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetIndices(ScriptRendererMeshData* self)
	{
		Vector<uint32_t> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = MeshDataEx::GetIndices(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<uint32_t>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetIndices(ScriptRendererMeshData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<uint32_t> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<uint32_t>(elementIndex);
			}
		}
		MeshDataEx::SetIndices(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	int32_t ScriptRendererMeshData::InternalGetVertexCount(ScriptRendererMeshData* self)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = MeshDataEx::GetVertexCount(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRendererMeshData::InternalGetIndexCount(ScriptRendererMeshData* self)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = MeshDataEx::GetIndexCount(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
}
