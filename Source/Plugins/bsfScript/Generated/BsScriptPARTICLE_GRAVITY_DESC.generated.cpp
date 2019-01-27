//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_GRAVITY_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptPARTICLE_GRAVITY_DESC::ScriptPARTICLE_GRAVITY_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_GRAVITY_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_GRAVITY_DESC::box(const PARTICLE_GRAVITY_DESC& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	PARTICLE_GRAVITY_DESC ScriptPARTICLE_GRAVITY_DESC::unbox(MonoObject* value)
	{
		return *(PARTICLE_GRAVITY_DESC*)MonoUtil::unbox(value);
	}

}
