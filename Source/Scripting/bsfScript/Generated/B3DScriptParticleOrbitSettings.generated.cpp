//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleOrbitSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"

namespace b3d
{
	ScriptParticleOrbitSettings::ScriptParticleOrbitSettings()
	{ }

	MonoObject* ScriptParticleOrbitSettings::Box(const __ParticleOrbitSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleOrbitSettingsInterop ScriptParticleOrbitSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleOrbitSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleOrbitSettings ScriptParticleOrbitSettings::FromInterop(const __ParticleOrbitSettingsInterop& value)
	{
		ParticleOrbitSettings output;
		TShared<TDistribution<TVector3<float>>> tmpCenter;
		ScriptVector3Distribution* scriptObjectWrapperCenter;
		scriptObjectWrapperCenter = ScriptVector3Distribution::GetScriptObjectWrapper(value.Center);
		if(scriptObjectWrapperCenter != nullptr)
			tmpCenter = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrapperCenter->GetBaseNativeObjectAsShared());
		if(tmpCenter != nullptr)
		output.Center = *tmpCenter;
		TShared<TDistribution<TVector3<float>>> tmpVelocity;
		ScriptVector3Distribution* scriptObjectWrapperVelocity;
		scriptObjectWrapperVelocity = ScriptVector3Distribution::GetScriptObjectWrapper(value.Velocity);
		if(scriptObjectWrapperVelocity != nullptr)
			tmpVelocity = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrapperVelocity->GetBaseNativeObjectAsShared());
		if(tmpVelocity != nullptr)
		output.Velocity = *tmpVelocity;
		TShared<TDistribution<float>> tmpRadial;
		ScriptFloatDistribution* scriptObjectWrapperRadial;
		scriptObjectWrapperRadial = ScriptFloatDistribution::GetScriptObjectWrapper(value.Radial);
		if(scriptObjectWrapperRadial != nullptr)
			tmpRadial = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrapperRadial->GetBaseNativeObjectAsShared());
		if(tmpRadial != nullptr)
		output.Radial = *tmpRadial;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__ParticleOrbitSettingsInterop ScriptParticleOrbitSettings::ToInterop(const ParticleOrbitSettings& value)
	{
		__ParticleOrbitSettingsInterop output;
		MonoObject* tmpCenter;
		TShared<TDistribution<TVector3<float>>> tmpCentercopy;
		tmpCentercopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Center);
		tmpCenter = ScriptVector3Distribution::GetOrCreateScriptObject(tmpCentercopy);
		output.Center = tmpCenter;
		MonoObject* tmpVelocity;
		TShared<TDistribution<TVector3<float>>> tmpVelocitycopy;
		tmpVelocitycopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Velocity);
		tmpVelocity = ScriptVector3Distribution::GetOrCreateScriptObject(tmpVelocitycopy);
		output.Velocity = tmpVelocity;
		MonoObject* tmpRadial;
		TShared<TDistribution<float>> tmpRadialcopy;
		tmpRadialcopy = B3DMakeShared<TDistribution<float>>(value.Radial);
		tmpRadial = ScriptFloatDistribution::GetOrCreateScriptObject(tmpRadialcopy);
		output.Radial = tmpRadial;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

}
