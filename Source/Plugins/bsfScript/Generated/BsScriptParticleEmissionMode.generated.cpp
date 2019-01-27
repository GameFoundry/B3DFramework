//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmissionMode.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptParticleEmissionMode::ScriptParticleEmissionMode(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleEmissionMode::initRuntimeData()
	{ }

	MonoObject*ScriptParticleEmissionMode::box(const ParticleEmissionMode& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	ParticleEmissionMode ScriptParticleEmissionMode::unbox(MonoObject* value)
	{
		return *(ParticleEmissionMode*)MonoUtil::unbox(value);
	}

}
