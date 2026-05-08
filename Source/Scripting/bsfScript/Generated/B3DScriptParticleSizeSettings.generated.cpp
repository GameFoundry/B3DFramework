//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleSizeSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "B3DScriptTDistribution.generated.h"

namespace b3d
{
	ScriptParticleSizeSettings::ScriptParticleSizeSettings()
	{ }

	MonoObject* ScriptParticleSizeSettings::Box(const __ParticleSizeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleSizeSettingsInterop ScriptParticleSizeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleSizeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleSizeSettings ScriptParticleSizeSettings::FromInterop(const __ParticleSizeSettingsInterop& value)
	{
		ParticleSizeSettings output;
		TShared<TDistribution<float>> tmpSize;
		ScriptFloatDistribution* scriptObjectWrapperSize;
		scriptObjectWrapperSize = ScriptFloatDistribution::GetScriptObjectWrapper(value.Size);
		if(scriptObjectWrapperSize != nullptr)
			tmpSize = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrapperSize->GetBaseNativeObjectAsShared());
		if(tmpSize != nullptr)
		output.Size = *tmpSize;
		TShared<TDistribution<TVector3<float>>> tmpSize3D;
		ScriptVector3Distribution* scriptObjectWrapperSize3D;
		scriptObjectWrapperSize3D = ScriptVector3Distribution::GetScriptObjectWrapper(value.Size3D);
		if(scriptObjectWrapperSize3D != nullptr)
			tmpSize3D = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrapperSize3D->GetBaseNativeObjectAsShared());
		if(tmpSize3D != nullptr)
		output.Size3D = *tmpSize3D;
		output.Use3DSize = value.Use3DSize;

		return output;
	}

	__ParticleSizeSettingsInterop ScriptParticleSizeSettings::ToInterop(const ParticleSizeSettings& value)
	{
		__ParticleSizeSettingsInterop output;
		MonoObject* tmpSize;
		TShared<TDistribution<float>> tmpSizecopy;
		tmpSizecopy = B3DMakeShared<TDistribution<float>>(value.Size);
		tmpSize = ScriptFloatDistribution::GetOrCreateScriptObject(tmpSizecopy);
		output.Size = tmpSize;
		MonoObject* tmpSize3D;
		TShared<TDistribution<TVector3<float>>> tmpSize3Dcopy;
		tmpSize3Dcopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Size3D);
		tmpSize3D = ScriptVector3Distribution::GetOrCreateScriptObject(tmpSize3Dcopy);
		output.Size3D = tmpSize3D;
		output.Use3DSize = value.Use3DSize;

		return output;
	}

}
