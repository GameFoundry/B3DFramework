//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRendererMeshData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "BsScriptRendererMeshData.generated.h"
#include "BsScriptBoneWeight.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Extensions/BsMeshDataEx.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	ScriptRendererMeshData::ScriptRendererMeshData(MonoObject* managedInstance, const SPtr<RendererMeshData>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptRendererMeshData::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptRendererMeshData::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_getPositions", (void*)&ScriptRendererMeshData::Internal_getPositions);
		metaData.scriptClass->addInternalCall("Internal_setPositions", (void*)&ScriptRendererMeshData::Internal_setPositions);
		metaData.scriptClass->addInternalCall("Internal_getNormals", (void*)&ScriptRendererMeshData::Internal_getNormals);
		metaData.scriptClass->addInternalCall("Internal_setNormals", (void*)&ScriptRendererMeshData::Internal_setNormals);
		metaData.scriptClass->addInternalCall("Internal_getTangents", (void*)&ScriptRendererMeshData::Internal_getTangents);
		metaData.scriptClass->addInternalCall("Internal_setTangents", (void*)&ScriptRendererMeshData::Internal_setTangents);
		metaData.scriptClass->addInternalCall("Internal_getColors", (void*)&ScriptRendererMeshData::Internal_getColors);
		metaData.scriptClass->addInternalCall("Internal_setColors", (void*)&ScriptRendererMeshData::Internal_setColors);
		metaData.scriptClass->addInternalCall("Internal_getUV0", (void*)&ScriptRendererMeshData::Internal_getUV0);
		metaData.scriptClass->addInternalCall("Internal_setUV0", (void*)&ScriptRendererMeshData::Internal_setUV0);
		metaData.scriptClass->addInternalCall("Internal_getUV1", (void*)&ScriptRendererMeshData::Internal_getUV1);
		metaData.scriptClass->addInternalCall("Internal_setUV1", (void*)&ScriptRendererMeshData::Internal_setUV1);
		metaData.scriptClass->addInternalCall("Internal_getBoneWeights", (void*)&ScriptRendererMeshData::Internal_getBoneWeights);
		metaData.scriptClass->addInternalCall("Internal_setBoneWeights", (void*)&ScriptRendererMeshData::Internal_setBoneWeights);
		metaData.scriptClass->addInternalCall("Internal_getIndices", (void*)&ScriptRendererMeshData::Internal_getIndices);
		metaData.scriptClass->addInternalCall("Internal_setIndices", (void*)&ScriptRendererMeshData::Internal_setIndices);
		metaData.scriptClass->addInternalCall("Internal_getVertexCount", (void*)&ScriptRendererMeshData::Internal_getVertexCount);
		metaData.scriptClass->addInternalCall("Internal_getIndexCount", (void*)&ScriptRendererMeshData::Internal_getIndexCount);

	}

	MonoObject* ScriptRendererMeshData::create(const SPtr<RendererMeshData>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptRendererMeshData>()) ScriptRendererMeshData(managedInstance, value);
		return managedInstance;
	}
	void ScriptRendererMeshData::Internal_create(MonoObject* managedInstance, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType)
	{
		SPtr<RendererMeshData> instance = MeshDataEx::create(numVertices, numIndices, layout, indexType);
		new (bs_alloc<ScriptRendererMeshData>())ScriptRendererMeshData(managedInstance, instance);
	}

	MonoArray* ScriptRendererMeshData::Internal_getPositions(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector3> vec__output;
		vec__output = MeshDataEx::getPositions(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptVector3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setPositions(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector3> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<Vector3>(i);
			}
		}
		MeshDataEx::setPositions(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::Internal_getNormals(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector3> vec__output;
		vec__output = MeshDataEx::getNormals(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptVector3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setNormals(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector3> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<Vector3>(i);
			}
		}
		MeshDataEx::setNormals(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::Internal_getTangents(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector4> vec__output;
		vec__output = MeshDataEx::getTangents(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptVector4>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setTangents(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector4> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<Vector4>(i);
			}
		}
		MeshDataEx::setTangents(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::Internal_getColors(ScriptRendererMeshData* thisPtr)
	{
		Vector<Color> vec__output;
		vec__output = MeshDataEx::getColors(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptColor>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setColors(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Color> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<Color>(i);
			}
		}
		MeshDataEx::setColors(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::Internal_getUV0(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector2> vec__output;
		vec__output = MeshDataEx::getUV0(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptVector2>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setUV0(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector2> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<Vector2>(i);
			}
		}
		MeshDataEx::setUV0(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::Internal_getUV1(ScriptRendererMeshData* thisPtr)
	{
		Vector<Vector2> vec__output;
		vec__output = MeshDataEx::getUV1(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptVector2>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setUV1(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<Vector2> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<Vector2>(i);
			}
		}
		MeshDataEx::setUV1(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::Internal_getBoneWeights(ScriptRendererMeshData* thisPtr)
	{
		Vector<BoneWeight> vec__output;
		vec__output = MeshDataEx::getBoneWeights(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptBoneWeight>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setBoneWeights(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<BoneWeight> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<BoneWeight>(i);
			}
		}
		MeshDataEx::setBoneWeights(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptRendererMeshData::Internal_getIndices(ScriptRendererMeshData* thisPtr)
	{
		Vector<uint32_t> vec__output;
		vec__output = MeshDataEx::getIndices(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<uint32_t>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptRendererMeshData::Internal_setIndices(ScriptRendererMeshData* thisPtr, MonoArray* value)
	{
		Vector<uint32_t> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<uint32_t>(i);
			}
		}
		MeshDataEx::setIndices(thisPtr->getInternal(), vecvalue);
	}

	int32_t ScriptRendererMeshData::Internal_getVertexCount(ScriptRendererMeshData* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::getVertexCount(thisPtr->getInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRendererMeshData::Internal_getIndexCount(ScriptRendererMeshData* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = MeshDataEx::getIndexCount(thisPtr->getInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
}
