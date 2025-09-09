//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleColorSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTColorDistribution.generated.h"

namespace b3d
{
	ScriptParticleColorOptions::ScriptParticleColorOptions()
	{ }

	MonoObject* ScriptParticleColorOptions::Box(const __ParticleColorSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleColorSettingsInterop ScriptParticleColorOptions::Unbox(MonoObject* value)
	{
		return *(__ParticleColorSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleColorSettings ScriptParticleColorOptions::FromInterop(const __ParticleColorSettingsInterop& value)
	{
		ParticleColorSettings output;
		SPtr<TColorDistribution<ColorGradient>> tmpColor;
		ScriptColorDistribution* scriptObjectWrapperColor;
		scriptObjectWrapperColor = ScriptColorDistribution::GetScriptObjectWrapper(value.Color);
		if(scriptObjectWrapperColor != nullptr)
			tmpColor = std::static_pointer_cast<TColorDistribution<ColorGradient>>(scriptObjectWrapperColor->GetBaseNativeObjectAsShared());
		if(tmpColor != nullptr)
		output.Color = *tmpColor;

		return output;
	}

	__ParticleColorSettingsInterop ScriptParticleColorOptions::ToInterop(const ParticleColorSettings& value)
	{
		__ParticleColorSettingsInterop output;
		MonoObject* tmpColor;
		SPtr<TColorDistribution<ColorGradient>> tmpColorcopy;
		tmpColorcopy = B3DMakeShared<TColorDistribution<ColorGradient>>(value.Color);
		tmpColor = ScriptColorDistribution::GetOrCreateScriptObject(tmpColorcopy);
		output.Color = tmpColor;

		return output;
	}

}
