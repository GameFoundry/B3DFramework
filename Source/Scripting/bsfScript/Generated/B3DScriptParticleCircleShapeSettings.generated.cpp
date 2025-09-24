//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleCircleShapeSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "B3DScriptParticleEmissionMode.generated.h"

namespace b3d
{
	ScriptParticleCircleShapeSettings::ScriptParticleCircleShapeSettings()
	{ }

	MonoObject* ScriptParticleCircleShapeSettings::Box(const __ParticleCircleShapeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleCircleShapeSettingsInterop ScriptParticleCircleShapeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleCircleShapeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleCircleShapeSettings ScriptParticleCircleShapeSettings::FromInterop(const __ParticleCircleShapeSettingsInterop& value)
	{
		ParticleCircleShapeSettings output;
		output.Radius = value.Radius;
		output.Thickness = value.Thickness;
		output.Arc = value.Arc;
		output.Mode = value.Mode;

		return output;
	}

	__ParticleCircleShapeSettingsInterop ScriptParticleCircleShapeSettings::ToInterop(const ParticleCircleShapeSettings& value)
	{
		__ParticleCircleShapeSettingsInterop output;
		output.Radius = value.Radius;
		output.Thickness = value.Thickness;
		output.Arc = value.Arc;
		output.Mode = value.Mode;

		return output;
	}

}
