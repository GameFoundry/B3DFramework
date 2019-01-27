//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_COLLISIONS_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptPARTICLE_COLLISIONS_DESC::ScriptPARTICLE_COLLISIONS_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_COLLISIONS_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_COLLISIONS_DESC::box(const PARTICLE_COLLISIONS_DESC& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	PARTICLE_COLLISIONS_DESC ScriptPARTICLE_COLLISIONS_DESC::unbox(MonoObject* value)
	{
		return *(PARTICLE_COLLISIONS_DESC*)MonoUtil::unbox(value);
	}

}
