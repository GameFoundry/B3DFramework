//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::ScriptPARTICLE_STATIC_MESH_SHAPE_DESC(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::InitRuntimeData()
	{}

	MonoObject* ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::Box(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_STATIC_MESH_SHAPE_DESCInterop ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_STATIC_MESH_SHAPE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_STATIC_MESH_SHAPE_DESC ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::FromInterop(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		ResourceHandle<Mesh> tmpMesh;
		ScriptRRefBase* scriptMesh;
		scriptMesh = ScriptRRefBase::ToNative(value.Mesh);
		if(scriptMesh != nullptr)
			tmpMesh = static_resource_cast<Mesh>(scriptMesh->GetHandle());
		output.Mesh = tmpMesh;

		return output;
	}

	__PARTICLE_STATIC_MESH_SHAPE_DESCInterop ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::ToInterop(const PARTICLE_STATIC_MESH_SHAPE_DESC& value)
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

} // namespace bs
