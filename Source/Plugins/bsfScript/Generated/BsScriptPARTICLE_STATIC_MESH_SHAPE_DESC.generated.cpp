//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_STATIC_MESH_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "BsScriptMesh.generated.h"

namespace bs
{
	ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::ScriptPARTICLE_STATIC_MESH_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::box(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_STATIC_MESH_SHAPE_DESCInterop ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_STATIC_MESH_SHAPE_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_STATIC_MESH_SHAPE_DESC ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::fromInterop(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC output;
		output.type = value.type;
		output.sequential = value.sequential;
		ResourceHandle<Mesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(value.mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<Mesh>(scriptmesh->getHandle());
		output.mesh = tmpmesh;

		return output;
	}

	__PARTICLE_STATIC_MESH_SHAPE_DESCInterop ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::toInterop(const PARTICLE_STATIC_MESH_SHAPE_DESC& value)
	{
		__PARTICLE_STATIC_MESH_SHAPE_DESCInterop output;
		output.type = value.type;
		output.sequential = value.sequential;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptResourceManager::instance().getScriptRRef(value.mesh);
		MonoObject* tmpmesh;
		if(scriptmesh != nullptr)
			tmpmesh = scriptmesh->getManagedInstance();
		else
			tmpmesh = nullptr;
		output.mesh = tmpmesh;

		return output;
	}

}
