//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_STATIC_MESH_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "BsScriptMesh.generated.h"

namespace bs
{
	ScriptParticleStaticMeshShapeOptions::ScriptParticleStaticMeshShapeOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleStaticMeshShapeOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleStaticMeshShapeOptions::Box(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_STATIC_MESH_SHAPE_DESCInterop ScriptParticleStaticMeshShapeOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_STATIC_MESH_SHAPE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_STATIC_MESH_SHAPE_DESC ScriptParticleStaticMeshShapeOptions::FromInterop(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		TResourceHandle<Mesh> tmpMesh;
		ScriptRRefBase* scriptMesh;
		scriptMesh = ScriptRRefBase::ToNative(value.Mesh);
		if(scriptMesh != nullptr)
			tmpMesh = B3DStaticResourceCast<Mesh>(scriptMesh->GetHandle());
		output.Mesh = tmpMesh;

		return output;
	}

	__PARTICLE_STATIC_MESH_SHAPE_DESCInterop ScriptParticleStaticMeshShapeOptions::ToInterop(const PARTICLE_STATIC_MESH_SHAPE_DESC& value)
	{
		__PARTICLE_STATIC_MESH_SHAPE_DESCInterop output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		MonoObject* tmpMesh;
		ScriptRRefBase* scriptMesh;
		scriptMesh = ScriptResourceManager::Instance().GetScriptRRef(value.Mesh);
		if(scriptMesh != nullptr)
			tmpMesh = scriptMesh->GetManagedInstance();
		else
			tmpMesh = nullptr;
		output.Mesh = tmpMesh;

		return output;
	}

}
