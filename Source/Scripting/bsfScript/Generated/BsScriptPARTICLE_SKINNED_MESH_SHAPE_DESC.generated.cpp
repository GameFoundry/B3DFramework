//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_SKINNED_MESH_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCRenderable.h"
#include "BsScriptCRenderable.generated.h"

namespace bs
{
	ScriptParticleSkinnedMeshShapeOptions::ScriptParticleSkinnedMeshShapeOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleSkinnedMeshShapeOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleSkinnedMeshShapeOptions::Box(const __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop ScriptParticleSkinnedMeshShapeOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_SKINNED_MESH_SHAPE_DESC ScriptParticleSkinnedMeshShapeOptions::FromInterop(const __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop& value)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		GameObjectHandle<CRenderable> tmpRenderable;
		ScriptRenderable* scriptWrapperObjectRenderable;
		scriptWrapperObjectRenderable = ScriptRenderable::GetScriptObjectWrapper(value.Renderable);
		if(scriptWrapperObjectRenderable != nullptr)
			tmpRenderable = B3DStaticGameObjectCast<CRenderable>(scriptWrapperObjectRenderable->GetBaseNativeObjectAsHandle());
		output.Renderable = tmpRenderable;

		return output;
	}

	__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop ScriptParticleSkinnedMeshShapeOptions::ToInterop(const PARTICLE_SKINNED_MESH_SHAPE_DESC& value)
	{
		__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		MonoObject* tmpRenderable;
		MonoObject* temptmpRenderable = nullptr;
		if(value.Renderable.GetComponent())
			temptmpRenderable = ScriptComponent::GetOrCreateScriptObject(value.Renderable.GetComponent());
		tmpRenderable = temptmpRenderable;
		output.Renderable = tmpRenderable;

		return output;
	}

}
