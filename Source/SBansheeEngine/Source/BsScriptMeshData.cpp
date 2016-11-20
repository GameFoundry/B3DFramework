//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptMeshData.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptColor.h"
#include "BsScriptVector.h"
#include "BsScriptBoneWeight.h"
#include "BsVertexDataDesc.h"
#include "BsPixelUtil.h"

namespace bs
{
	template<int Semantic>
	struct TVertexDataAccessor
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size) { }
		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size) { }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Position >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getPositions((Vector3*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setPositions((Vector3*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Normal >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getNormals((Vector3*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setNormals((Vector3*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Tangent >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getTangents((Vector4*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setTangents((Vector4*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::Color >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getColors((Color*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setColors((Color*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::UV0 >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getUV0((Vector2*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setUV0((Vector2*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::UV1 >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getUV1((Vector2*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setUV1((Vector2*)buffer, size); }
	};

	template<>
	struct TVertexDataAccessor < (int)VertexLayout::BoneWeights >
	{
		static void get(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->getBoneWeights((BoneWeight*)buffer, size); }

		static void set(const SPtr<RendererMeshData>& meshData, UINT8* buffer, UINT32 size)
		{ meshData->setBoneWeights((BoneWeight*)buffer, size); }
	};

	template<int Semantic, class TNative, class TScript>
	MonoArray* getVertexDataArray(ScriptMeshData* scriptMeshData)
	{
		SPtr<RendererMeshData> meshData = scriptMeshData->getInternalValue();
		UINT32 numElements = meshData->getData()->getNumVertices();

		ScriptArray outArray = ScriptArray::create<TScript>(numElements);
		TVertexDataAccessor<Semantic>::get(meshData, (UINT8*)outArray.getRawPtr<TNative>(), numElements * sizeof(TNative));

		return outArray.getInternal();
	}

	template<int Semantic, class TNative, class TScript>
	void setVertexDataArray(ScriptMeshData* scriptMeshData, MonoArray* array)
	{
		if (array == nullptr)
			return;

		SPtr<RendererMeshData> meshData = scriptMeshData->getInternalValue();
		UINT32 numElements = meshData->getData()->getNumVertices();

		ScriptArray inArray(array);
		TVertexDataAccessor<Semantic>::set(meshData, (UINT8*)inArray.getRawPtr<TNative>(), numElements * sizeof(TNative));
	}

	ScriptMeshData::ScriptMeshData(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{

	}

	ScriptMeshData::~ScriptMeshData()
	{

	}

	void ScriptMeshData::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptMeshData::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPositions", &ScriptMeshData::internal_GetPositions);
		metaData.scriptClass->addInternalCall("Internal_SetPositions", &ScriptMeshData::internal_SetPositions);
		metaData.scriptClass->addInternalCall("Internal_GetNormals", &ScriptMeshData::internal_GetNormals);
		metaData.scriptClass->addInternalCall("Internal_SetNormals", &ScriptMeshData::internal_SetNormals);
		metaData.scriptClass->addInternalCall("Internal_GetTangents", &ScriptMeshData::internal_GetTangents);
		metaData.scriptClass->addInternalCall("Internal_SetTangents", &ScriptMeshData::internal_SetTangents);
		metaData.scriptClass->addInternalCall("Internal_GetColors", &ScriptMeshData::internal_GetColors);
		metaData.scriptClass->addInternalCall("Internal_SetColors", &ScriptMeshData::internal_SetColors);
		metaData.scriptClass->addInternalCall("Internal_GetUV0", &ScriptMeshData::internal_GetUV0);
		metaData.scriptClass->addInternalCall("Internal_SetUV0", &ScriptMeshData::internal_SetUV0);
		metaData.scriptClass->addInternalCall("Internal_GetUV1", &ScriptMeshData::internal_GetUV1);
		metaData.scriptClass->addInternalCall("Internal_SetUV1", &ScriptMeshData::internal_SetUV1);
		metaData.scriptClass->addInternalCall("Internal_GetBoneWeights", &ScriptMeshData::internal_GetBoneWeights);
		metaData.scriptClass->addInternalCall("Internal_SetBoneWeights", &ScriptMeshData::internal_SetBoneWeights);
		metaData.scriptClass->addInternalCall("Internal_GetIndices", &ScriptMeshData::internal_GetIndices);
		metaData.scriptClass->addInternalCall("Internal_SetIndices", &ScriptMeshData::internal_SetIndices);
		metaData.scriptClass->addInternalCall("Internal_GetVertexCount", &ScriptMeshData::internal_GetVertexCount);
		metaData.scriptClass->addInternalCall("Internal_GetIndexCount", &ScriptMeshData::internal_GetIndexCount);
	}

	void ScriptMeshData::initialize(const SPtr<RendererMeshData>& meshData)
	{
		mMeshData = meshData;
	}

	MonoObject* ScriptMeshData::create(const SPtr<RendererMeshData>& meshData)
	{
		MonoObject* meshDataObj = metaData.scriptClass->createInstance();

		ScriptMeshData* scriptMeshData = new (bs_alloc<ScriptMeshData>()) ScriptMeshData(meshDataObj);
		scriptMeshData->initialize(meshData);

		return meshDataObj;
	}

	MonoObject* ScriptMeshData::create(const SPtr<MeshData>& meshData)
	{
		MonoObject* meshDataObj = metaData.scriptClass->createInstance();

		SPtr<RendererMeshData> defaultMeshData = RendererMeshData::create(meshData);
		ScriptMeshData* scriptMeshData = new (bs_alloc<ScriptMeshData>()) ScriptMeshData(meshDataObj);
		scriptMeshData->initialize(defaultMeshData);

		return meshDataObj;
	}

	void ScriptMeshData::internal_CreateInstance(MonoObject* instance, int numVertices,
		int numIndices, VertexLayout vertex, ScriptIndexType index)
	{
		IndexType indexType = IT_16BIT;
		if (index == ScriptIndexType::Index32)
			indexType = IT_32BIT;

		SPtr<RendererMeshData> meshData = RendererMeshData::create(numVertices, numIndices, vertex, indexType);

		ScriptMeshData* scriptMeshData = new (bs_alloc<ScriptMeshData>()) ScriptMeshData(instance);
		scriptMeshData->initialize(meshData);
	}

	MonoArray* ScriptMeshData::internal_GetPositions(ScriptMeshData* thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Position, Vector3, ScriptVector3>(thisPtr);
	}

	void ScriptMeshData::internal_SetPositions(ScriptMeshData* thisPtr, MonoArray* value)
	{
		setVertexDataArray<(int)VertexLayout::Position, Vector3, ScriptVector3>(thisPtr, value);
	}

	MonoArray* ScriptMeshData::internal_GetNormals(ScriptMeshData* thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Normal, Vector3, ScriptVector3>(thisPtr);
	}

	void ScriptMeshData::internal_SetNormals(ScriptMeshData* thisPtr, MonoArray* value)
	{
		setVertexDataArray<(int)VertexLayout::Normal, Vector3, ScriptVector3>(thisPtr, value);
	}

	MonoArray* ScriptMeshData::internal_GetTangents(ScriptMeshData* thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Tangent, Vector4, ScriptVector4>(thisPtr);
	}

	void ScriptMeshData::internal_SetTangents(ScriptMeshData* thisPtr, MonoArray* value)
	{
		setVertexDataArray<(int)VertexLayout::Tangent, Vector4, ScriptVector4>(thisPtr, value);
	}

	MonoArray* ScriptMeshData::internal_GetColors(ScriptMeshData* thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::Color, Color, ScriptColor>(thisPtr);
	}

	void ScriptMeshData::internal_SetColors(ScriptMeshData* thisPtr, MonoArray* value)
	{
		setVertexDataArray<(int)VertexLayout::Color, Color, ScriptColor>(thisPtr, value);
	}

	MonoArray* ScriptMeshData::internal_GetUV0(ScriptMeshData* thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::UV0, Vector2, ScriptVector2>(thisPtr);
	}

	void ScriptMeshData::internal_SetUV0(ScriptMeshData* thisPtr, MonoArray* value)
	{
		setVertexDataArray<(int)VertexLayout::UV0, Vector2, ScriptVector2>(thisPtr, value);
	}

	MonoArray* ScriptMeshData::internal_GetUV1(ScriptMeshData* thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::UV1, Vector2, ScriptVector2>(thisPtr);
	}

	void ScriptMeshData::internal_SetUV1(ScriptMeshData* thisPtr, MonoArray* value)
	{
		setVertexDataArray<(int)VertexLayout::UV1, Vector2, ScriptVector2>(thisPtr, value);
	}

	MonoArray* ScriptMeshData::internal_GetBoneWeights(ScriptMeshData* thisPtr)
	{
		return getVertexDataArray<(int)VertexLayout::BoneWeights, BoneWeight, ScriptBoneWeight>(thisPtr);
	}

	void ScriptMeshData::internal_SetBoneWeights(ScriptMeshData* thisPtr, MonoArray* value)
	{
		setVertexDataArray<(int)VertexLayout::BoneWeights, BoneWeight, ScriptBoneWeight>(thisPtr, value);
	}

	MonoArray* ScriptMeshData::internal_GetIndices(ScriptMeshData* thisPtr)
	{
		SPtr<RendererMeshData> meshData = thisPtr->getInternalValue();
		UINT32 numElements = meshData->getData()->getNumIndices();

		ScriptArray outArray = ScriptArray::create<UINT32>(numElements);
		meshData->getIndices(outArray.getRawPtr<UINT32>(), numElements * sizeof(UINT32));

		return outArray.getInternal();
	}

	void ScriptMeshData::internal_SetIndices(ScriptMeshData* thisPtr, MonoArray* value)
	{
		if (value == nullptr)
			return;

		SPtr<RendererMeshData> meshData = thisPtr->getInternalValue();
		UINT32 numElements = meshData->getData()->getNumIndices();

		ScriptArray inArray(value);
		meshData->setIndices(inArray.getRawPtr<UINT32>(), numElements * sizeof(UINT32));
	}

	int ScriptMeshData::internal_GetVertexCount(ScriptMeshData* thisPtr)
	{
		SPtr<RendererMeshData> meshData = thisPtr->getInternalValue();

		return (int)meshData->getData()->getNumVertices();
	}

	int ScriptMeshData::internal_GetIndexCount(ScriptMeshData* thisPtr)
	{
		SPtr<RendererMeshData> meshData = thisPtr->getInternalValue();

		return (int)meshData->getData()->getNumIndices();
	}

	bool ScriptMeshData::checkIsLocked(ScriptMeshData* thisPtr)
	{
		if (thisPtr->mMeshData->getData()->isLocked())
		{
			LOGWRN("Attempting to access a locked mesh data buffer.");
			return true;
		}

		return false;
	}
}