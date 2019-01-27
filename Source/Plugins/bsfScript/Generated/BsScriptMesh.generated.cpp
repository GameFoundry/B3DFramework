//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMesh.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptSkeleton.generated.h"
#include "BsScriptRendererMeshData.generated.h"
#include "BsScriptMorphShapes.generated.h"
#include "BsScriptSubMesh.generated.h"
#include "../../../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "../../../Extensions/BsMeshEx.h"

namespace bs
{
	ScriptMesh::ScriptMesh(MonoObject* managedInstance, const ResourceHandle<Mesh>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptMesh::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRef", (void*)&ScriptMesh::Internal_getRef);
		metaData.scriptClass->addInternalCall("Internal_getSkeleton", (void*)&ScriptMesh::Internal_getSkeleton);
		metaData.scriptClass->addInternalCall("Internal_getMorphShapes", (void*)&ScriptMesh::Internal_getMorphShapes);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptMesh::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptMesh::Internal_create0);
		metaData.scriptClass->addInternalCall("Internal_create1", (void*)&ScriptMesh::Internal_create1);
		metaData.scriptClass->addInternalCall("Internal_create2", (void*)&ScriptMesh::Internal_create2);
		metaData.scriptClass->addInternalCall("Internal_getSubMeshes", (void*)&ScriptMesh::Internal_getSubMeshes);
		metaData.scriptClass->addInternalCall("Internal_getSubMeshCount", (void*)&ScriptMesh::Internal_getSubMeshCount);
		metaData.scriptClass->addInternalCall("Internal_getBounds", (void*)&ScriptMesh::Internal_getBounds);
		metaData.scriptClass->addInternalCall("Internal_getMeshData", (void*)&ScriptMesh::Internal_getMeshData);
		metaData.scriptClass->addInternalCall("Internal_setMeshData", (void*)&ScriptMesh::Internal_setMeshData);

	}

	 MonoObject*ScriptMesh::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->createInstance("bool", ctorParams);
	}
	MonoObject* ScriptMesh::Internal_getRef(ScriptMesh* thisPtr)
	{
		return thisPtr->getRRef();
	}

	MonoObject* ScriptMesh::Internal_getSkeleton(ScriptMesh* thisPtr)
	{
		SPtr<Skeleton> tmp__output;
		tmp__output = thisPtr->getHandle()->getSkeleton();

		MonoObject* __output;
		__output = ScriptSkeleton::create(tmp__output);

		return __output;
	}

	MonoObject* ScriptMesh::Internal_getMorphShapes(ScriptMesh* thisPtr)
	{
		SPtr<MorphShapes> tmp__output;
		tmp__output = thisPtr->getHandle()->getMorphShapes();

		MonoObject* __output;
		__output = ScriptMorphShapes::create(tmp__output);

		return __output;
	}

	void ScriptMesh::Internal_create(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		ResourceHandle<Mesh> instance = MeshEx::create(numVertices, numIndices, topology, usage, vertex, index);
		ScriptResourceManager::instance().createBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMesh::Internal_create0(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index)
	{
		Vector<SubMesh> vecsubMeshes;
		if(subMeshes != nullptr)
		{
			ScriptArray arraysubMeshes(subMeshes);
			vecsubMeshes.resize(arraysubMeshes.size());
			for(int i = 0; i < (int)arraysubMeshes.size(); i++)
			{
				vecsubMeshes[i] = arraysubMeshes.get<SubMesh>(i);
			}

		}
		ResourceHandle<Mesh> instance = MeshEx::create(numVertices, numIndices, vecsubMeshes, usage, vertex, index);
		ScriptResourceManager::instance().createBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMesh::Internal_create1(MonoObject* managedInstance, MonoObject* data, DrawOperationType topology, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptRendererMeshData* scriptdata;
		scriptdata = ScriptRendererMeshData::toNative(data);
		if(scriptdata != nullptr)
			tmpdata = scriptdata->getInternal();
		ResourceHandle<Mesh> instance = MeshEx::create(tmpdata, topology, usage);
		ScriptResourceManager::instance().createBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMesh::Internal_create2(MonoObject* managedInstance, MonoObject* data, MonoArray* subMeshes, MeshUsage usage)
	{
		SPtr<RendererMeshData> tmpdata;
		ScriptRendererMeshData* scriptdata;
		scriptdata = ScriptRendererMeshData::toNative(data);
		if(scriptdata != nullptr)
			tmpdata = scriptdata->getInternal();
		Vector<SubMesh> vecsubMeshes;
		if(subMeshes != nullptr)
		{
			ScriptArray arraysubMeshes(subMeshes);
			vecsubMeshes.resize(arraysubMeshes.size());
			for(int i = 0; i < (int)arraysubMeshes.size(); i++)
			{
				vecsubMeshes[i] = arraysubMeshes.get<SubMesh>(i);
			}

		}
		ResourceHandle<Mesh> instance = MeshEx::create(tmpdata, vecsubMeshes, usage);
		ScriptResourceManager::instance().createBuiltinScriptResource(instance, managedInstance);
	}

	MonoArray* ScriptMesh::Internal_getSubMeshes(ScriptMesh* thisPtr)
	{
		Vector<SubMesh> vec__output;
		vec__output = MeshEx::getSubMeshes(thisPtr->getHandle());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptSubMesh>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	uint32_t ScriptMesh::Internal_getSubMeshCount(ScriptMesh* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = MeshEx::getSubMeshCount(thisPtr->getHandle());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMesh::Internal_getBounds(ScriptMesh* thisPtr, AABox* box, Sphere* sphere)
	{
		MeshEx::getBounds(thisPtr->getHandle(), box, sphere);
	}

	MonoObject* ScriptMesh::Internal_getMeshData(ScriptMesh* thisPtr)
	{
		SPtr<RendererMeshData> tmp__output;
		tmp__output = MeshEx::getMeshData(thisPtr->getHandle());

		MonoObject* __output;
		__output = ScriptRendererMeshData::create(tmp__output);

		return __output;
	}

	void ScriptMesh::Internal_setMeshData(ScriptMesh* thisPtr, MonoObject* value)
	{
		SPtr<RendererMeshData> tmpvalue;
		ScriptRendererMeshData* scriptvalue;
		scriptvalue = ScriptRendererMeshData::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->getInternal();
		MeshEx::setMeshData(thisPtr->getHandle(), tmpvalue);
	}
}
