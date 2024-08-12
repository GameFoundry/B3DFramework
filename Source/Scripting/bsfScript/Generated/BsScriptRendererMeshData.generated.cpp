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
		SPtr<RendererMeshData> instance = MeshDataEx::Create(numVertices, numIndices, layout, indexType);
		new (B3DAllocate<ScriptMeshData>())ScriptMeshData(managedInstance, instance);
	}

	MonoArray* ScriptMeshData::InternalGetPositions(ScriptMeshData* thisPtr)
	{
		Vector<TVector3<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetPositions(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetPositions(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetPositions(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetNormals(ScriptMeshData* thisPtr)
	{
		Vector<TVector3<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetNormals(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetNormals(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetNormals(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetTangents(ScriptMeshData* thisPtr)
	{
		Vector<TVector4<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetTangents(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetTangents(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetTangents(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetColors(ScriptMeshData* thisPtr)
	{
		Vector<Color> nativeArray__output;
		nativeArray__output = MeshDataEx::GetColors(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetColors(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetColors(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetUV0(ScriptMeshData* thisPtr)
	{
		Vector<TVector2<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetUV0(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetUV0(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetUV0(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetUV1(ScriptMeshData* thisPtr)
	{
		Vector<TVector2<float>> nativeArray__output;
		nativeArray__output = MeshDataEx::GetUV1(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetUV1(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetUV1(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetBoneWeights(ScriptMeshData* thisPtr)
	{
		Vector<BoneWeight> nativeArray__output;
		nativeArray__output = MeshDataEx::GetBoneWeights(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetBoneWeights(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetBoneWeights(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptMeshData::InternalGetIndices(ScriptMeshData* thisPtr)
	{
		Vector<uint32_t> nativeArray__output;
		nativeArray__output = MeshDataEx::GetIndices(thisPtr->GetInternal());

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

	void ScriptMeshData::InternalSetIndices(ScriptMeshData* thisPtr, MonoArray* value)
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
		MeshDataEx::SetIndices(thisPtr->GetInternal(), nativeArrayvalue);
	}

	int32_t ScriptMeshData::InternalGetVertexCount(ScriptMeshData* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetVertexCount(thisPtr->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptMeshData::InternalGetIndexCount(ScriptMeshData* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetIndexCount(thisPtr->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
}
