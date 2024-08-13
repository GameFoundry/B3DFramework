//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRendererMeshData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptRendererMeshData.generated.h"
#include "../Extensions/BsMeshDataEx.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptColor.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptBoneWeight.generated.h"

namespace bs
{
	ScriptMeshData::ScriptMeshData(MonoObject* managedInstance, const SPtr<RendererMeshData>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptMeshData::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMeshData::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_GetPositions", (void*)&ScriptMeshData::InternalGetPositions);
		metaData.ScriptClass->AddInternalCall("Internal_SetPositions", (void*)&ScriptMeshData::InternalSetPositions);
		metaData.ScriptClass->AddInternalCall("Internal_GetNormals", (void*)&ScriptMeshData::InternalGetNormals);
		metaData.ScriptClass->AddInternalCall("Internal_SetNormals", (void*)&ScriptMeshData::InternalSetNormals);
		metaData.ScriptClass->AddInternalCall("Internal_GetTangents", (void*)&ScriptMeshData::InternalGetTangents);
		metaData.ScriptClass->AddInternalCall("Internal_SetTangents", (void*)&ScriptMeshData::InternalSetTangents);
		metaData.ScriptClass->AddInternalCall("Internal_GetColors", (void*)&ScriptMeshData::InternalGetColors);
		metaData.ScriptClass->AddInternalCall("Internal_SetColors", (void*)&ScriptMeshData::InternalSetColors);
		metaData.ScriptClass->AddInternalCall("Internal_GetUV0", (void*)&ScriptMeshData::InternalGetUV0);
		metaData.ScriptClass->AddInternalCall("Internal_SetUV0", (void*)&ScriptMeshData::InternalSetUV0);
		metaData.ScriptClass->AddInternalCall("Internal_GetUV1", (void*)&ScriptMeshData::InternalGetUV1);
		metaData.ScriptClass->AddInternalCall("Internal_SetUV1", (void*)&ScriptMeshData::InternalSetUV1);
		metaData.ScriptClass->AddInternalCall("Internal_GetBoneWeights", (void*)&ScriptMeshData::InternalGetBoneWeights);
		metaData.ScriptClass->AddInternalCall("Internal_SetBoneWeights", (void*)&ScriptMeshData::InternalSetBoneWeights);
		metaData.ScriptClass->AddInternalCall("Internal_GetIndices", (void*)&ScriptMeshData::InternalGetIndices);
		metaData.ScriptClass->AddInternalCall("Internal_SetIndices", (void*)&ScriptMeshData::InternalSetIndices);
		metaData.ScriptClass->AddInternalCall("Internal_GetVertexCount", (void*)&ScriptMeshData::InternalGetVertexCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetIndexCount", (void*)&ScriptMeshData::InternalGetIndexCount);

	}

	MonoObject* ScriptMeshData::Create(const SPtr<RendererMeshData>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptMeshData>()) ScriptMeshData(managedInstance, value);
		return managedInstance;
	}
	void ScriptMeshData::InternalCreate(MonoObject* managedInstance, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType)
	{
		SPtr<RendererMeshData> nativeObject = MeshDataEx::Create(numVertices, numIndices, layout, indexType);
		new (B3DAllocate<ScriptMeshData>())ScriptMeshData(managedInstance, nativeObject);
	}

	MonoArray* ScriptMeshData::InternalGetPositions(ScriptMeshData* self)
	{
		Vector<TVector3<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetPositions(self->GetInternal());

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
		MeshDataEx::SetPositions(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetNormals(ScriptMeshData* self)
	{
		Vector<TVector3<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetNormals(self->GetInternal());

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
		MeshDataEx::SetNormals(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetTangents(ScriptMeshData* self)
	{
		Vector<TVector4<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetTangents(self->GetInternal());

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
		MeshDataEx::SetTangents(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetColors(ScriptMeshData* self)
	{
		Vector<Color> nativeArray__output;
		nativeArray__output = MeshDataEx::GetColors(self->GetInternal());

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
		MeshDataEx::SetColors(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetUV0(ScriptMeshData* self)
	{
		Vector<TVector2<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetUV0(self->GetInternal());

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
		MeshDataEx::SetUV0(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetUV1(ScriptMeshData* self)
	{
		Vector<TVector2<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetUV1(self->GetInternal());

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
		MeshDataEx::SetUV1(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetBoneWeights(ScriptMeshData* self)
	{
		Vector<BoneWeight> nativeArray__output;
		nativeArray__output = MeshDataEx::GetBoneWeights(self->GetInternal());

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
		MeshDataEx::SetBoneWeights(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetIndices(ScriptMeshData* self)
	{
		Vector<uint32_t> nativeArray__output;
		nativeArray__output = MeshDataEx::GetIndices(self->GetInternal());

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
		MeshDataEx::SetIndices(self->GetInternal(), nativeArrayvalue);
	}

	int32_t ScriptMeshData::InternalGetVertexCount(ScriptMeshData* self)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetVertexCount(self->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptMeshData::InternalGetIndexCount(ScriptMeshData* self)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetIndexCount(self->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
}
