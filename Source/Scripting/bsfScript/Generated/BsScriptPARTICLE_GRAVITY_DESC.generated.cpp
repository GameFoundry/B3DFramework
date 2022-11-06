//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	void ScriptPARTICLE_GRAVITY_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_GRAVITY_DESC::Box(const PARTICLE_GRAVITY_DESC& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	PARTICLE_GRAVITY_DESC ScriptPARTICLE_GRAVITY_DESC::Unbox(MonoObject* value)
	{
		return *(PARTICLE_GRAVITY_DESC*)MonoUtil::Unbox(value);
	}

}
