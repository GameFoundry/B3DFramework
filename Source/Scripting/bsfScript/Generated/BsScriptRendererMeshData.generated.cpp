//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRendererMeshData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "BsScriptTVector3.generated.h"
#include "BsScriptRendererMeshData.generated.h"
#include "../Extensions/BsMeshDataEx.h"
#include "BsScriptTVector4.generated.h"
#include "Wrappers/BsScriptColor.h"
#include "BsScriptTVector2.generated.h"
#include "BsScriptBoneWeight.generated.h"

namespace bs
{
	ScriptMeshData::ScriptMeshData(const SPtr<RendererMeshData>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptMeshData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMeshData::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPositions", (void*)&ScriptMeshData::InternalGetPositions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPositions", (void*)&ScriptMeshData::InternalSetPositions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNormals", (void*)&ScriptMeshData::InternalGetNormals);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNormals", (void*)&ScriptMeshData::InternalSetNormals);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTangents", (void*)&ScriptMeshData::InternalGetTangents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTangents", (void*)&ScriptMeshData::InternalSetTangents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColors", (void*)&ScriptMeshData::InternalGetColors);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColors", (void*)&ScriptMeshData::InternalSetColors);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUV0", (void*)&ScriptMeshData::InternalGetUV0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUV0", (void*)&ScriptMeshData::InternalSetUV0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUV1", (void*)&ScriptMeshData::InternalGetUV1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUV1", (void*)&ScriptMeshData::InternalSetUV1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBoneWeights", (void*)&ScriptMeshData::InternalGetBoneWeights);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBoneWeights", (void*)&ScriptMeshData::InternalSetBoneWeights);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIndices", (void*)&ScriptMeshData::InternalGetIndices);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIndices", (void*)&ScriptMeshData::InternalSetIndices);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVertexCount", (void*)&ScriptMeshData::InternalGetVertexCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIndexCount", (void*)&ScriptMeshData::InternalGetIndexCount);

	}

	MonoObject* ScriptMeshData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptMeshData::InternalCreate(MonoObject* scriptObject, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType)
	{
		SPtr<RendererMeshData> nativeObject = MeshDataEx::Create(numVertices, numIndices, layout, indexType);
		ScriptObjectWrapper::Create<ScriptMeshData>(nativeObject, scriptObject);
	}

	MonoArray* ScriptMeshData::InternalGetPositions(ScriptMeshData* self)
	{
		Vector<TVector3<float>> nativeArray__output;
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

	void ScriptMeshData::InternalSetPositions(ScriptMeshData* self, MonoArray* value)
	{
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

	MonoArray* ScriptMeshData::InternalGetNormals(ScriptMeshData* self)
	{
		Vector<TVector3<float>> nativeArray__output;
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

	void ScriptMeshData::InternalSetNormals(ScriptMeshData* self, MonoArray* value)
	{
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

	MonoArray* ScriptMeshData::InternalGetTangents(ScriptMeshData* self)
	{
		Vector<TVector4<float>> nativeArray__output;
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

	void ScriptMeshData::InternalSetTangents(ScriptMeshData* self, MonoArray* value)
	{
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

	MonoArray* ScriptMeshData::InternalGetColors(ScriptMeshData* self)
	{
		Vector<Color> nativeArray__output;
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

	void ScriptMeshData::InternalSetColors(ScriptMeshData* self, MonoArray* value)
	{
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

	MonoArray* ScriptMeshData::InternalGetUV0(ScriptMeshData* self)
	{
		Vector<TVector2<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetUV0(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptVector2>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptMeshData::InternalSetUV0(ScriptMeshData* self, MonoArray* value)
	{
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

	MonoArray* ScriptMeshData::InternalGetUV1(ScriptMeshData* self)
	{
		Vector<TVector2<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetUV1(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptVector2>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptMeshData::InternalSetUV1(ScriptMeshData* self, MonoArray* value)
	{
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

	MonoArray* ScriptMeshData::InternalGetBoneWeights(ScriptMeshData* self)
	{
		Vector<BoneWeight> nativeArray__output;
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

	void ScriptMeshData::InternalSetBoneWeights(ScriptMeshData* self, MonoArray* value)
	{
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

	MonoArray* ScriptMeshData::InternalGetIndices(ScriptMeshData* self)
	{
		Vector<uint32_t> nativeArray__output;
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

	void ScriptMeshData::InternalSetIndices(ScriptMeshData* self, MonoArray* value)
	{
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

	int32_t ScriptMeshData::InternalGetVertexCount(ScriptMeshData* self)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetVertexCount(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptMeshData::InternalGetIndexCount(ScriptMeshData* self)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetIndexCount(std::static_pointer_cast<RendererMeshData>(self->GetBaseNativeObjectAsShared()));

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
}
