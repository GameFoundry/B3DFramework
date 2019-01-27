//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_SKINNED_MESH_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "../../../../../Foundation/bsfCore/Components/BsCRenderable.h"
#include "BsScriptCRenderable.generated.h"

namespace bs
{
	ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::box(const __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_SKINNED_MESH_SHAPE_DESC ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::fromInterop(const __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop& value)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC output;
		output.type = value.type;
		output.sequential = value.sequential;
		GameObjectHandle<CRenderable> tmprenderable;
		ScriptCRenderable* scriptrenderable;
		scriptrenderable = ScriptCRenderable::toNative(value.renderable);
		if(scriptrenderable != nullptr)
			tmprenderable = scriptrenderable->getHandle();
		output.renderable = tmprenderable;

		return output;
	}

	__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::toInterop(const PARTICLE_SKINNED_MESH_SHAPE_DESC& value)
	{
		__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop output;
		output.type = value.type;
		output.sequential = value.sequential;
		ScriptComponentBase* scriptrenderable = nullptr;
		if(value.renderable.getComponent())
			scriptrenderable = ScriptGameObjectManager::instance().getBuiltinScriptComponent(static_object_cast<Component>(value.renderable.getComponent()));
		MonoObject* tmprenderable;
		if(scriptrenderable != nullptr)
			tmprenderable = scriptrenderable->getManagedInstance();
		else
			tmprenderable = nullptr;
		output.renderable = tmprenderable;

		return output;
	}

}
