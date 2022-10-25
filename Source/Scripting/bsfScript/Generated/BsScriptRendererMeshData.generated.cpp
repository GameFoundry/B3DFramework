//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRendererMeshData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "BsScriptRendererMeshData.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "../Extensions/BsMeshDataEx.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptColor.h"
#include "BsScriptBoneWeight.generated.h"

namespace bs
{
	ScriptRendererMeshData::ScriptRendererMeshData(MonoObject* managedInstance, const SPtr<RendererMeshData>& value)
		: ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptRendererMeshData::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptRendererMeshData::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_GetPositions", (void*)&ScriptRendererMeshData::InternalGetPositions);
		metaData.ScriptClass->AddInternalCall("Internal_SetPositions", (void*)&ScriptRendererMeshData::InternalSetPositions);
		metaData.ScriptClass->AddInternalCall("Internal_GetNormals", (void*)&ScriptRendererMeshData::InternalGetNormals);
		metaData.ScriptClass->AddInternalCall("Internal_SetNormals", (void*)&ScriptRendererMeshData::InternalSetNormals);
		metaData.ScriptClass->AddInternalCall("Internal_GetTangents", (void*)&ScriptRendererMeshData::InternalGetTangents);
		metaData.ScriptClass->AddInternalCall("Internal_SetTangents", (void*)&ScriptRendererMeshData::InternalSetTangents);
		metaData.ScriptClass->AddInternalCall("Internal_GetColors", (void*)&ScriptRendererMeshData::InternalGetColors);
		metaData.ScriptClass->AddInternalCall("Internal_SetColors", (void*)&ScriptRendererMeshData::InternalSetColors);
		metaData.ScriptClass->AddInternalCall("Internal_GetUV0", (void*)&ScriptRendererMeshData::InternalGetUV0);
		metaData.ScriptClass->AddInternalCall("Internal_SetUV0", (void*)&ScriptRendererMeshData::InternalSetUV0);
		metaData.ScriptClass->AddInternalCall("Internal_GetUV1", (void*)&ScriptRendererMeshData::InternalGetUV1);
		metaData.ScriptClass->AddInternalCall("Internal_SetUV1", (void*)&ScriptRendererMeshData::InternalSetUV1);
		metaData.ScriptClass->AddInternalCall("Internal_GetBoneWeights", (void*)&ScriptRendererMeshData::InternalGetBoneWeights);
		metaData.ScriptClass->AddInternalCall("Internal_SetBoneWeights", (void*)&ScriptRendererMeshData::InternalSetBoneWeights);
		metaData.ScriptClass->AddInternalCall("Internal_GetIndices", (void*)&ScriptRendererMeshData::InternalGetIndices);
		metaData.ScriptClass->AddInternalCall("Internal_SetIndices", (void*)&ScriptRendererMeshData::InternalSetIndices);
		metaData.ScriptClass->AddInternalCall("Internal_GetVertexCount", (void*)&ScriptRendererMeshData::InternalGetVertexCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetIndexCount", (void*)&ScriptRendererMeshData::InternalGetIndexCount);
	}

	MonoObject* ScriptRendererMeshData::Create(const SPtr<RendererMeshData>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptRendererMeshData>()) ScriptRendererMeshData(managedInstance, value);
		return managedInstance;
	}

	void ScriptRendererMeshData::InternalCreate(MonoObject* managedInstance, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType)
	{
		SPtr<RendererMeshData> instance = MeshDataEx::Create(numVertices, numIndices, layout, indexType);
		new(bs_alloc<ScriptRendererMeshData>()) ScriptRendererMeshData(managedInstance, instance);
	}

	MonoArray* ScriptRendererMeshData::InternalGetPositions(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector3> vec__output;
		vec__output = MeshDataEx::GetPositions(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptVector3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetPositions(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector3> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<Vector3>(i);
			}
		}
		MeshDataEx::SetPositions(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetNormals(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector3> vec__output;
		vec__output = MeshDataEx::GetNormals(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptVector3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetNormals(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector3> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<Vector3>(i);
			}
		}
		MeshDataEx::SetNormals(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetTangents(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector4> vec__output;
		vec__output = MeshDataEx::GetTangents(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptVector4>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetTangents(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector4> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<Vector4>(i);
			}
		}
		MeshDataEx::SetTangents(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetColors(ScriptRendererMeshData* thisPtr)
	{
		Vector<Color> vec__output;
		vec__output = MeshDataEx::GetColors(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptColor>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetColors(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Color> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<Color>(i);
			}
		}
		MeshDataEx::SetColors(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetUV0(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector2> vec__output;
		vec__output = MeshDataEx::GetUV0(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptVector2>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetUV0(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector2> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<Vector2>(i);
			}
		}
		MeshDataEx::SetUV0(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetUV1(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector2> vec__output;
		vec__output = MeshDataEx::GetUV1(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptVector2>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetUV1(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector2> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<Vector2>(i);
			}
		}
		MeshDataEx::SetUV1(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetBoneWeights(ScriptRendererMeshData* thisPtr)
	{
		Vector<BoneWeight> vec__output;
		vec__output = MeshDataEx::GetBoneWeights(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptBoneWeight>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetBoneWeights(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<BoneWeight> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<BoneWeight>(i);
			}
		}
		MeshDataEx::SetBoneWeights(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::InternalGetIndices(ScriptRendererMeshData* thisPtr)
	{
		Vector<uint32_t> vec__output;
		vec__output = MeshDataEx::GetIndices(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<uint32_t>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRendererMeshData::InternalSetIndices(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<uint32_t> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = arrayvalue.Get<uint32_t>(i);
			}
		}
		MeshDataEx::SetIndices(thisPtr->GetInternal(), vecvalue);
	}

	int32_t ScriptRendererMeshData::InternalGetVertexCount(ScriptRendererMeshData* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetVertexCount(thisPtr->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRendererMeshData::InternalGetIndexCount(ScriptRendererMeshData* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::GetIndexCount(thisPtr->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
} // namespace bs
