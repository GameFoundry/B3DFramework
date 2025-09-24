//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleLineShapeSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "B3DScriptParticleEmissionMode.generated.h"

namespace b3d
{
	ScriptParticleLineShapeSettings::ScriptParticleLineShapeSettings()
	{ }

	MonoObject* ScriptParticleLineShapeSettings::Box(const __ParticleLineShapeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleLineShapeSettingsInterop ScriptParticleLineShapeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleLineShapeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleLineShapeSettings ScriptParticleLineShapeSettings::FromInterop(const __ParticleLineShapeSettingsInterop& value)
	{
		ParticleLineShapeSettings output;
		output.Length = value.Length;
		output.Mode = value.Mode;

		return output;
	}

	__ParticleLineShapeSettingsInterop ScriptParticleLineShapeSettings::ToInterop(const ParticleLineShapeSettings& value)
	{
		__ParticleLineShapeSettingsInterop output;
		output.Length = value.Length;
		output.Mode = value.Mode;

		return output;
	}

}
