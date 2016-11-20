//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPhysicsMesh.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsScriptMeshData.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace bs
{
	ScriptPhysicsMesh::ScriptPhysicsMesh(MonoObject* instance, const HPhysicsMesh& mesh)
		:TScriptResource(instance, mesh)
	{

	}

	void ScriptPhysicsMesh::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetMeshData", &ScriptPhysicsMesh::internal_GetMeshData);
		metaData.scriptClass->addInternalCall("Internal_GetMeshType", &ScriptPhysicsMesh::internal_GetMeshType);
	}

	MonoObject* ScriptPhysicsMesh::internal_GetMeshData(ScriptPhysicsMesh* thisPtr)
	{
		HPhysicsMesh mesh = thisPtr->getHandle();

		SPtr<MeshData> meshData = mesh->getMeshData();
		return ScriptMeshData::create(meshData);
	}

	int ScriptPhysicsMesh::internal_GetMeshType(ScriptPhysicsMesh* thisPtr)
	{
		HPhysicsMesh mesh = thisPtr->getHandle();
		return (int)mesh->getType();
	}

	MonoObject* ScriptPhysicsMesh::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}