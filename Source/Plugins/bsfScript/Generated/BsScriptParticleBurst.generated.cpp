//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleBurst.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptParticleBurst::ScriptParticleBurst(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleBurst::initRuntimeData()
	{ }

	MonoObject*ScriptParticleBurst::box(const __ParticleBurstInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__ParticleBurstInterop ScriptParticleBurst::unbox(MonoObject* value)
	{
		return *(__ParticleBurstInterop*)MonoUtil::unbox(value);
	}

	ParticleBurst ScriptParticleBurst::fromInterop(const __ParticleBurstInterop& value)
	{
		ParticleBurst output;
		output.time = value.time;
		SPtr<TDistribution<float>> tmpcount;
		ScriptTDistributionfloat* scriptcount;
		scriptcount = ScriptTDistributionfloat::toNative(value.count);
		if(scriptcount != nullptr)
			tmpcount = scriptcount->getInternal();
		if(tmpcount != nullptr)
		output.count = *tmpcount;
		output.cycles = value.cycles;
		output.interval = value.interval;

		return output;
	}

	__ParticleBurstInterop ScriptParticleBurst::toInterop(const ParticleBurst& value)
	{
		__ParticleBurstInterop output;
		output.time = value.time;
		MonoObject* tmpcount;
		SPtr<TDistribution<float>> tmpcountcopy;
		tmpcountcopy = bs_shared_ptr_new<TDistribution<float>>(value.count);
		tmpcount = ScriptTDistributionfloat::create(tmpcountcopy);
		output.count = tmpcount;
		output.cycles = value.cycles;
		output.interval = value.interval;

		return output;
	}

}
