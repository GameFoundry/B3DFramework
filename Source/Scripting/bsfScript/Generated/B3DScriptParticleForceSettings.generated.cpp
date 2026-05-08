//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleForceSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"

namespace b3d
{
	ScriptParticleForceSettings::ScriptParticleForceSettings()
	{ }

	MonoObject* ScriptParticleForceSettings::Box(const __ParticleForceSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleForceSettingsInterop ScriptParticleForceSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleForceSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleForceSettings ScriptParticleForceSettings::FromInterop(const __ParticleForceSettingsInterop& value)
	{
		ParticleForceSettings output;
		TShared<TDistribution<TVector3<float>>> tmpForce;
		ScriptVector3Distribution* scriptObjectWrapperForce;
		scriptObjectWrapperForce = ScriptVector3Distribution::GetScriptObjectWrapper(value.Force);
		if(scriptObjectWrapperForce != nullptr)
			tmpForce = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrapperForce->GetBaseNativeObjectAsShared());
		if(tmpForce != nullptr)
		output.Force = *tmpForce;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__ParticleForceSettingsInterop ScriptParticleForceSettings::ToInterop(const ParticleForceSettings& value)
	{
		__ParticleForceSettingsInterop output;
		MonoObject* tmpForce;
		TShared<TDistribution<TVector3<float>>> tmpForcecopy;
		tmpForcecopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Force);
		tmpForce = ScriptVector3Distribution::GetOrCreateScriptObject(tmpForcecopy);
		output.Force = tmpForce;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

}
