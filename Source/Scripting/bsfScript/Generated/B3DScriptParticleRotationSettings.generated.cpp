//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleRotationSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"

namespace b3d
{
	ScriptParticleRotationSettings::ScriptParticleRotationSettings()
	{ }

	MonoObject* ScriptParticleRotationSettings::Box(const __ParticleRotationSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleRotationSettingsInterop ScriptParticleRotationSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleRotationSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleRotationSettings ScriptParticleRotationSettings::FromInterop(const __ParticleRotationSettingsInterop& value)
	{
		ParticleRotationSettings output;
		TShared<TDistribution<float>> tmpRotation;
		ScriptFloatDistribution* scriptObjectWrapperRotation;
		scriptObjectWrapperRotation = ScriptFloatDistribution::GetScriptObjectWrapper(value.Rotation);
		if(scriptObjectWrapperRotation != nullptr)
			tmpRotation = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrapperRotation->GetBaseNativeObjectAsShared());
		if(tmpRotation != nullptr)
		output.Rotation = *tmpRotation;
		TShared<TDistribution<TVector3<float>>> tmpRotation3D;
		ScriptVector3Distribution* scriptObjectWrapperRotation3D;
		scriptObjectWrapperRotation3D = ScriptVector3Distribution::GetScriptObjectWrapper(value.Rotation3D);
		if(scriptObjectWrapperRotation3D != nullptr)
			tmpRotation3D = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrapperRotation3D->GetBaseNativeObjectAsShared());
		if(tmpRotation3D != nullptr)
		output.Rotation3D = *tmpRotation3D;
		output.Use3DRotation = value.Use3DRotation;

		return output;
	}

	__ParticleRotationSettingsInterop ScriptParticleRotationSettings::ToInterop(const ParticleRotationSettings& value)
	{
		__ParticleRotationSettingsInterop output;
		MonoObject* tmpRotation;
		TShared<TDistribution<float>> tmpRotationcopy;
		tmpRotationcopy = B3DMakeShared<TDistribution<float>>(value.Rotation);
		tmpRotation = ScriptFloatDistribution::GetOrCreateScriptObject(tmpRotationcopy);
		output.Rotation = tmpRotation;
		MonoObject* tmpRotation3D;
		TShared<TDistribution<TVector3<float>>> tmpRotation3Dcopy;
		tmpRotation3Dcopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Rotation3D);
		tmpRotation3D = ScriptVector3Distribution::GetOrCreateScriptObject(tmpRotation3Dcopy);
		output.Rotation3D = tmpRotation3D;
		output.Use3DRotation = value.Use3DRotation;

		return output;
	}

}
