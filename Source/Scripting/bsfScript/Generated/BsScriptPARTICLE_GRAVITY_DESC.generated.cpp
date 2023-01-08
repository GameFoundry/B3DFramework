//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_GRAVITY_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptParticleGravityOptions::ScriptParticleGravityOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleGravityOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleGravityOptions::Box(const PARTICLE_GRAVITY_DESC& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	PARTICLE_GRAVITY_DESC ScriptParticleGravityOptions::Unbox(MonoObject* value)
	{
		return *(PARTICLE_GRAVITY_DESC*)MonoUtil::Unbox(value);
	}

}
