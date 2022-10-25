//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmissionMode.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptParticleEmissionMode::ScriptParticleEmissionMode(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptParticleEmissionMode::InitRuntimeData()
	{}

	MonoObject* ScriptParticleEmissionMode::Box(const ParticleEmissionMode& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	ParticleEmissionMode ScriptParticleEmissionMode::Unbox(MonoObject* value)
	{
		return *(ParticleEmissionMode*)MonoUtil::Unbox(value);
	}

} // namespace bs
