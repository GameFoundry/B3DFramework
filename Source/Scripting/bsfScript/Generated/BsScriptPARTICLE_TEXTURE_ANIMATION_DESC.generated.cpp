//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_TEXTURE_ANIMATION_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptPARTICLE_TEXTURE_ANIMATION_DESC::ScriptPARTICLE_TEXTURE_ANIMATION_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_TEXTURE_ANIMATION_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_TEXTURE_ANIMATION_DESC::Box(const PARTICLE_TEXTURE_ANIMATION_DESC& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	PARTICLE_TEXTURE_ANIMATION_DESC ScriptPARTICLE_TEXTURE_ANIMATION_DESC::Unbox(MonoObject* value)
	{
		return *(PARTICLE_TEXTURE_ANIMATION_DESC*)MonoUtil::Unbox(value);
	}

}
