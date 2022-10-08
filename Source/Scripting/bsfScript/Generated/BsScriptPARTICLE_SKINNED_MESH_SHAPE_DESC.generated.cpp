//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_SKINNED_MESH_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "../../../Foundation/bsfCore/Components/BsCRenderable.h"
#include "BsScriptCRenderable.generated.h"

namespace bs
{
	ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::Box(const __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_SKINNED_MESH_SHAPE_DESC ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::FromInterop(const __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop& value)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		GameObjectHandle<CRenderable> tmpRenderable;
		ScriptCRenderable* scriptRenderable;
		scriptRenderable = ScriptCRenderable::ToNative(value.Renderable);
		if(scriptRenderable != nullptr)
			tmpRenderable = scriptRenderable->GetHandle();
		output.Renderable = tmpRenderable;

		return output;
	}

	__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::ToInterop(const PARTICLE_SKINNED_MESH_SHAPE_DESC& value)
	{
		__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		MonoObject* tmpRenderable;
		ScriptComponentBase* scriptRenderable = nullptr;
		if(value.Renderable.GetComponent())
			scriptRenderable = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(value.Renderable.GetComponent()));
		if(scriptRenderable != nullptr)
			tmpRenderable = scriptRenderable->GetManagedInstance();
		else
			tmpRenderable = nullptr;
		output.Renderable = tmpRenderable;

		return output;
	}

}
