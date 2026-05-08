//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleBurst.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"

namespace b3d
{
	ScriptParticleBurst::ScriptParticleBurst()
	{ }

	MonoObject* ScriptParticleBurst::Box(const __ParticleBurstInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleBurstInterop ScriptParticleBurst::Unbox(MonoObject* value)
	{
		return *(__ParticleBurstInterop*)MonoUtil::Unbox(value);
	}

	ParticleBurst ScriptParticleBurst::FromInterop(const __ParticleBurstInterop& value)
	{
		ParticleBurst output;
		output.Time = value.Time;
		TShared<TDistribution<float>> tmpCount;
		ScriptFloatDistribution* scriptObjectWrapperCount;
		scriptObjectWrapperCount = ScriptFloatDistribution::GetScriptObjectWrapper(value.Count);
		if(scriptObjectWrapperCount != nullptr)
			tmpCount = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrapperCount->GetBaseNativeObjectAsShared());
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
		TShared<TDistribution<float>> tmpCountcopy;
		tmpCountcopy = B3DMakeShared<TDistribution<float>>(value.Count);
		tmpCount = ScriptFloatDistribution::GetOrCreateScriptObject(tmpCountcopy);
		output.Count = tmpCount;
		output.Cycles = value.Cycles;
		output.Interval = value.Interval;

		return output;
	}

}
