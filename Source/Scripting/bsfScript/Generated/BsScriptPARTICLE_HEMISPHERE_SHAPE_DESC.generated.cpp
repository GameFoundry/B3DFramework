//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_HEMISPHERE_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptPARTICLE_HEMISPHERE_SHAPE_DESC::ScriptPARTICLE_HEMISPHERE_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_HEMISPHERE_SHAPE_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_HEMISPHERE_SHAPE_DESC::Box(const PARTICLE_HEMISPHERE_SHAPE_DESC& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	PARTICLE_HEMISPHERE_SHAPE_DESC ScriptPARTICLE_HEMISPHERE_SHAPE_DESC::Unbox(MonoObject* value)
	{
		return *(PARTICLE_HEMISPHERE_SHAPE_DESC*)MonoUtil::Unbox(value);
	}

}
