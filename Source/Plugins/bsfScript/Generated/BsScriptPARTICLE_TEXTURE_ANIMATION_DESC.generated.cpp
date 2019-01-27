//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptPARTICLE_TEXTURE_ANIMATION_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_TEXTURE_ANIMATION_DESC::box(const PARTICLE_TEXTURE_ANIMATION_DESC& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	PARTICLE_TEXTURE_ANIMATION_DESC ScriptPARTICLE_TEXTURE_ANIMATION_DESC::unbox(MonoObject* value)
	{
		return *(PARTICLE_TEXTURE_ANIMATION_DESC*)MonoUtil::unbox(value);
	}

}
