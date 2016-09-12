//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptMesh.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoArray.h"
#include "BsMonoManager.h"
#include "BsCoreThread.h"
#include "BsScriptSkeleton.h"

namespace BansheeEngine
{
	ScriptSubMesh::ScriptSubMesh(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptSubMesh::initRuntimeData()
	{ }

	MonoObject* ScriptSubMesh::box(const SubMeshData& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	SubMeshData ScriptSubMesh::unbox(MonoObject* obj)
	{
		return *(SubMeshData*)MonoUtil::unbox(obj);
	}

	ScriptMesh::ScriptMesh(MonoObject* instance, const HMesh& mesh)
		:TScriptResource(instance, mesh)
	{

	}

	void ScriptMesh::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptMesh::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_CreateInstanceMeshData", &ScriptMesh::internal_CreateInstanceMeshData);
		metaData.scriptClass->addInternalCall("Internal_GetSubMeshes", &ScriptMesh::internal_GetSubMeshes);
		metaData.scriptClass->addInternalCall("Internal_GetSubMeshCount", &ScriptMesh::internal_GetSubMeshCount);
		metaData.scriptClass->addInternalCall("Internal_GetSkeleton", &ScriptMesh::internal_GetSkeleton);
		metaData.scriptClass->addInternalCall("Internal_GetBounds", &ScriptMesh::internal_GetBounds);
		metaData.scriptClass->addInternalCall("Internal_GetMeshData", &ScriptMesh::internal_GetMeshData);
		metaData.scriptClass->addInternalCall("Internal_SetMeshData", &ScriptMesh::internal_SetMeshData);
	}

	void ScriptMesh::internal_CreateInstance(MonoObject* instance, int numVertices, int numIndices, 
		MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, ScriptIndexType index)
	{
		IndexType indexType = IT_16BIT;
		if (index == ScriptIndexType::Index32)
			indexType = IT_32BIT;

		MESH_DESC desc;
		desc.numVertices = numVertices;
		desc.numIndices = numIndices;
		desc.vertexDesc = RendererMeshData::vertexLayoutVertexDesc(vertex);
		desc.subMeshes = monoToNativeSubMeshes(subMeshes);
		desc.usage = usage;
		desc.indexType = indexType;

		HMesh mesh = Mesh::create(desc);

		ScriptMesh* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, mesh, &scriptInstance);
	}

	void ScriptMesh::internal_CreateInstanceMeshData(MonoObject* instance, ScriptMeshData* data, MonoArray* subMeshes,
		MeshUsage usage)
	{
		SPtr<MeshData> meshData;
		if (data != nullptr)
			meshData = data->getInternalValue()->getData();

		MESH_DESC desc;
		desc.subMeshes = monoToNativeSubMeshes(subMeshes);
		desc.usage = usage;

		HMesh mesh = Mesh::create(meshData, desc);

		ScriptMesh* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, mesh, &scriptInstance);
	}

	MonoArray* ScriptMesh::internal_GetSubMeshes(ScriptMesh* thisPtr)
	{
		HMesh mesh = thisPtr->getHandle();

		UINT32 numSubMeshes = mesh->getProperties().getNumSubMeshes();
		ScriptArray subMeshArray = ScriptArray::create<ScriptSubMesh>(numSubMeshes);

		for (UINT32 i = 0; i < numSubMeshes; i++)
		{
			SubMesh curSubMesh = mesh->getProperties().getSubMesh(i);

			SubMeshData data;
			data.indexOffset = curSubMesh.indexOffset;
			data.indexCount = curSubMesh.indexCount;
			data.topology = drawOpToMeshTopology(curSubMesh.drawOp);

			subMeshArray.set(i, data);
		}

		return subMeshArray.getInternal();
	}

	UINT32 ScriptMesh::internal_GetSubMeshCount(ScriptMesh* thisPtr)
	{
		HMesh mesh = thisPtr->getHandle();

		return mesh->getProperties().getNumSubMeshes();
	}

	MonoObject* ScriptMesh::internal_GetSkeleton(ScriptMesh* thisPtr)
	{
		HMesh mesh = thisPtr->getHandle();

		SPtr<Skeleton> skeleton = mesh->getSkeleton();
		if (skeleton == nullptr)
			return nullptr;

		return ScriptSkeleton::create(skeleton);
	}

	void ScriptMesh::internal_GetBounds(ScriptMesh* thisPtr, AABox* box, Sphere* sphere)
	{
		HMesh mesh = thisPtr->getHandle();

		Bounds bounds = mesh->getProperties().getBounds();
		*box = bounds.getBox();
		*sphere = bounds.getSphere();
	}

	MonoObject* ScriptMesh::internal_GetMeshData(ScriptMesh* thisPtr)
	{
		HMesh mesh = thisPtr->getHandle();

		SPtr<MeshData> meshData = mesh->allocateSubresourceBuffer(0);
		mesh->readData(*meshData);

		return ScriptMeshData::create(meshData);
	}

	void ScriptMesh::internal_SetMeshData(ScriptMesh* thisPtr, ScriptMeshData* value)
	{
		HMesh mesh = thisPtr->getHandle();
		if (value != nullptr)
		{
			SPtr<MeshData> meshData = value->getInternalValue()->getData();
			mesh->writeSubresource(gCoreAccessor(), 0, meshData, true);
		}
	}

	MonoObject* ScriptMesh::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}

	DrawOperationType ScriptMesh::meshTopologyToDrawOp(MeshTopology topology)
	{
		switch (topology)
		{
		case MeshTopology::PointList:
			return DOT_POINT_LIST;
		case MeshTopology::LineList:
			return DOT_LINE_LIST;
		case MeshTopology::LineStrip:
			return DOT_LINE_STRIP;
		case MeshTopology::TriangleList:
			return DOT_TRIANGLE_LIST;
		case MeshTopology::TriangleStrip:
			return DOT_TRIANGLE_STRIP;
		case MeshTopology::TriangleFan:
			return DOT_TRIANGLE_FAN;
		}

		return DOT_TRIANGLE_LIST;
	}

	MeshTopology ScriptMesh::drawOpToMeshTopology(DrawOperationType drawOp)
	{
		switch (drawOp)
		{
		case DOT_POINT_LIST:
			return MeshTopology::PointList;
		case DOT_LINE_LIST:
			return MeshTopology::TriangleList;
		case DOT_LINE_STRIP:
			return MeshTopology::TriangleStrip;
		case DOT_TRIANGLE_LIST:
			return MeshTopology::TriangleList;
		case DOT_TRIANGLE_STRIP:
			return MeshTopology::TriangleStrip;
		case DOT_TRIANGLE_FAN:
			return MeshTopology::TriangleFan;
		}

		return MeshTopology::TriangleList;
	}

	Vector<SubMesh> ScriptMesh::monoToNativeSubMeshes(MonoArray* subMeshes)
	{
		Vector<SubMesh> nativeSubMeshes;

		if (subMeshes != nullptr)
		{
			ScriptArray subMeshArray(subMeshes);

			UINT32 numSubMeshes = subMeshArray.size();
			for (UINT32 i = 0; i < numSubMeshes; i++)
			{
				SubMeshData curData = subMeshArray.get<SubMeshData>(i);

				SubMesh subMesh;
				subMesh.indexOffset = curData.indexOffset;
				subMesh.indexCount = curData.indexCount;
				subMesh.drawOp = meshTopologyToDrawOp(curData.topology);

				nativeSubMeshes.push_back(subMesh);
			}
		}

		return nativeSubMeshes;
	}
}