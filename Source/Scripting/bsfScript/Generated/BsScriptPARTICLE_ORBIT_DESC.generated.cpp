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
	ScriptParticleOrbitOptions::ScriptParticleOrbitOptions()
	{ }

	MonoObject* ScriptParticleOrbitOptions::Box(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__PARTICLE_ORBIT_DESCInterop ScriptParticleOrbitOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_ORBIT_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_ORBIT_DESC ScriptParticleOrbitOptions::FromInterop(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		PARTICLE_ORBIT_DESC output;
		SPtr<TDistribution<TVector3<float>>> tmpCenter;
		ScriptVector3Distribution* scriptWrapperObjectCenter;
		scriptWrapperObjectCenter = ScriptVector3Distribution::GetScriptObjectWrapper(value.Center);
		if(scriptWrapperObjectCenter != nullptr)
			tmpCenter = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptWrapperObjectCenter->GetBaseNativeObjectAsShared());
		if(tmpCenter != nullptr)
		output.Center = *tmpCenter;
		SPtr<TDistribution<TVector3<float>>> tmpVelocity;
		ScriptVector3Distribution* scriptWrapperObjectVelocity;
		scriptWrapperObjectVelocity = ScriptVector3Distribution::GetScriptObjectWrapper(value.Velocity);
		if(scriptWrapperObjectVelocity != nullptr)
			tmpVelocity = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptWrapperObjectVelocity->GetBaseNativeObjectAsShared());
		if(tmpVelocity != nullptr)
		output.Velocity = *tmpVelocity;
		SPtr<TDistribution<float>> tmpRadial;
		ScriptFloatDistribution* scriptWrapperObjectRadial;
		scriptWrapperObjectRadial = ScriptFloatDistribution::GetScriptObjectWrapper(value.Radial);
		if(scriptWrapperObjectRadial != nullptr)
			tmpRadial = std::static_pointer_cast<TDistribution<float>>(scriptWrapperObjectRadial->GetBaseNativeObjectAsShared());
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
		tmpCenter = ScriptVector3Distribution::GetOrCreateScriptObject(tmpCentercopy);
		output.Center = tmpCenter;
		MonoObject* tmpVelocity;
		SPtr<TDistribution<TVector3<float>>> tmpVelocitycopy;
		tmpVelocitycopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Velocity);
		tmpVelocity = ScriptVector3Distribution::GetOrCreateScriptObject(tmpVelocitycopy);
		output.Velocity = tmpVelocity;
		MonoObject* tmpRadial;
		SPtr<TDistribution<float>> tmpRadialcopy;
		tmpRadialcopy = B3DMakeShared<TDistribution<float>>(value.Radial);
		tmpRadial = ScriptFloatDistribution::GetOrCreateScriptObject(tmpRadialcopy);
		output.Radial = tmpRadial;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

}
