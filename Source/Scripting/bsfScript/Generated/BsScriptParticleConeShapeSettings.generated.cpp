//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleConeShapeSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmissionMode.generated.h"

namespace b3d
{
	ScriptParticleConeShapeSettings::ScriptParticleConeShapeSettings()
	{ }

	MonoObject* ScriptParticleConeShapeSettings::Box(const __ParticleConeShapeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleConeShapeSettingsInterop ScriptParticleConeShapeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleConeShapeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleConeShapeSettings ScriptParticleConeShapeSettings::FromInterop(const __ParticleConeShapeSettingsInterop& value)
	{
		ParticleConeShapeSettings output;
		output.Type = value.Type;
		output.Radius = value.Radius;
		output.Angle = value.Angle;
		output.Length = value.Length;
		output.Thickness = value.Thickness;
		output.Arc = value.Arc;
		output.Mode = value.Mode;

		return output;
	}

	__ParticleConeShapeSettingsInterop ScriptParticleConeShapeSettings::ToInterop(const ParticleConeShapeSettings& value)
	{
		__ParticleConeShapeSettingsInterop output;
		output.Type = value.Type;
		output.Radius = value.Radius;
		output.Angle = value.Angle;
		output.Length = value.Length;
		output.Thickness = value.Thickness;
		output.Arc = value.Arc;
		output.Mode = value.Mode;

		return output;
	}

}
