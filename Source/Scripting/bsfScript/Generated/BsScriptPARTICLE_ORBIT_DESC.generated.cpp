//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_ORBIT_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptParticleOrbitOptions::ScriptParticleOrbitOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleOrbitOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleOrbitOptions::Box(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_ORBIT_DESCInterop ScriptParticleOrbitOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_ORBIT_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_ORBIT_DESC ScriptParticleOrbitOptions::FromInterop(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		PARTICLE_ORBIT_DESC output;
		SPtr<TDistribution<TVector3<float>>> tmpCenter;
		ScriptVector3Distribution* scriptCenter;
		scriptCenter = ScriptVector3Distribution::ToNative(value.Center);
		if(scriptCenter != nullptr)
			tmpCenter = scriptCenter->GetInternal();
		if(tmpCenter != nullptr)
		output.Center = *tmpCenter;
		SPtr<TDistribution<TVector3<float>>> tmpVelocity;
		ScriptVector3Distribution* scriptVelocity;
		scriptVelocity = ScriptVector3Distribution::ToNative(value.Velocity);
		if(scriptVelocity != nullptr)
			tmpVelocity = scriptVelocity->GetInternal();
		if(tmpVelocity != nullptr)
		output.Velocity = *tmpVelocity;
		SPtr<TDistribution<float>> tmpRadial;
		ScriptFloatDistribution* scriptRadial;
		scriptRadial = ScriptFloatDistribution::ToNative(value.Radial);
		if(scriptRadial != nullptr)
			tmpRadial = scriptRadial->GetInternal();
		if(tmpRadial != nullptr)
		output.Radial = *tmpRadial;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__PARTICLE_ORBIT_DESCInterop ScriptParticleOrbitOptions::ToInterop(const PARTICLE_ORBIT_DESC& value)
	{
		__PARTICLE_ORBIT_DESCInterop output;
		MonoObject* tmpCenter;
		SPtr<TDistribution<TVector3<float>>> tmpCentercopy;
		tmpCentercopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Center);
		tmpCenter = ScriptVector3Distribution::Create(tmpCentercopy);
		output.Center = tmpCenter;
		MonoObject* tmpVelocity;
		SPtr<TDistribution<TVector3<float>>> tmpVelocitycopy;
		tmpVelocitycopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Velocity);
		tmpVelocity = ScriptVector3Distribution::Create(tmpVelocitycopy);
		output.Velocity = tmpVelocity;
		MonoObject* tmpRadial;
		SPtr<TDistribution<float>> tmpRadialcopy;
		tmpRadialcopy = B3DMakeShared<TDistribution<float>>(value.Radial);
		tmpRadial = ScriptFloatDistribution::Create(tmpRadialcopy);
		output.Radial = tmpRadial;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

}
