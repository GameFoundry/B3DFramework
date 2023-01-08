//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_VELOCITY_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptParticleVelocityOptions::ScriptParticleVelocityOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleVelocityOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleVelocityOptions::Box(const __PARTICLE_VELOCITY_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_VELOCITY_DESCInterop ScriptParticleVelocityOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_VELOCITY_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_VELOCITY_DESC ScriptParticleVelocityOptions::FromInterop(const __PARTICLE_VELOCITY_DESCInterop& value)
	{
		PARTICLE_VELOCITY_DESC output;
		SPtr<TDistribution<Vector3>> tmpVelocity;
		ScriptVector3Distribution* scriptVelocity;
		scriptVelocity = ScriptVector3Distribution::ToNative(value.Velocity);
		if(scriptVelocity != nullptr)
			tmpVelocity = scriptVelocity->GetInternal();
		if(tmpVelocity != nullptr)
		output.Velocity = *tmpVelocity;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__PARTICLE_VELOCITY_DESCInterop ScriptParticleVelocityOptions::ToInterop(const PARTICLE_VELOCITY_DESC& value)
	{
		__PARTICLE_VELOCITY_DESCInterop output;
		MonoObject* tmpVelocity;
		SPtr<TDistribution<Vector3>> tmpVelocitycopy;
		tmpVelocitycopy = B3DMakeShared<TDistribution<Vector3>>(value.Velocity);
		tmpVelocity = ScriptVector3Distribution::Create(tmpVelocitycopy);
		output.Velocity = tmpVelocity;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

}
