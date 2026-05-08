//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleVelocitySettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"

namespace b3d
{
	ScriptParticleVelocitySettings::ScriptParticleVelocitySettings()
	{ }

	MonoObject* ScriptParticleVelocitySettings::Box(const __ParticleVelocitySettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleVelocitySettingsInterop ScriptParticleVelocitySettings::Unbox(MonoObject* value)
	{
		return *(__ParticleVelocitySettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleVelocitySettings ScriptParticleVelocitySettings::FromInterop(const __ParticleVelocitySettingsInterop& value)
	{
		ParticleVelocitySettings output;
		TShared<TDistribution<TVector3<float>>> tmpVelocity;
		ScriptVector3Distribution* scriptObjectWrapperVelocity;
		scriptObjectWrapperVelocity = ScriptVector3Distribution::GetScriptObjectWrapper(value.Velocity);
		if(scriptObjectWrapperVelocity != nullptr)
			tmpVelocity = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrapperVelocity->GetBaseNativeObjectAsShared());
		if(tmpVelocity != nullptr)
		output.Velocity = *tmpVelocity;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__ParticleVelocitySettingsInterop ScriptParticleVelocitySettings::ToInterop(const ParticleVelocitySettings& value)
	{
		__ParticleVelocitySettingsInterop output;
		MonoObject* tmpVelocity;
		TShared<TDistribution<TVector3<float>>> tmpVelocitycopy;
		tmpVelocitycopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Velocity);
		tmpVelocity = ScriptVector3Distribution::GetOrCreateScriptObject(tmpVelocitycopy);
		output.Velocity = tmpVelocity;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

}
