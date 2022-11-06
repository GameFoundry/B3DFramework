//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleBurst.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptParticleBurst::ScriptParticleBurst(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleBurst::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleBurst::Box(const __ParticleBurstInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ParticleBurstInterop ScriptParticleBurst::Unbox(MonoObject* value)
	{
		return *(__ParticleBurstInterop*)MonoUtil::Unbox(value);
	}

	ParticleBurst ScriptParticleBurst::FromInterop(const __ParticleBurstInterop& value)
	{
		ParticleBurst output;
		output.Time = value.Time;
		SPtr<TDistribution<float>> tmpCount;
		ScriptTDistributionfloat* scriptCount;
		scriptCount = ScriptTDistributionfloat::ToNative(value.Count);
		if(scriptCount != nullptr)
			tmpCount = scriptCount->GetInternal();
		if(tmpCount != nullptr)
		output.Count = *tmpCount;
		output.Cycles = value.Cycles;
		output.Interval = value.Interval;

		return output;
	}

	__ParticleBurstInterop ScriptParticleBurst::ToInterop(const ParticleBurst& value)
	{
		__ParticleBurstInterop output;
		output.Time = value.Time;
		MonoObject* tmpCount;
		SPtr<TDistribution<float>> tmpCountcopy;
		tmpCountcopy = B3DMakeShared<TDistribution<float>>(value.Count);
		tmpCount = ScriptTDistributionfloat::Create(tmpCountcopy);
		output.Count = tmpCount;
		output.Cycles = value.Cycles;
		output.Interval = value.Interval;

		return output;
	}

}
