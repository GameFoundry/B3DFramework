//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleBoxShapeSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptParticleBoxShapeSettings::ScriptParticleBoxShapeSettings()
	{ }

	MonoObject* ScriptParticleBoxShapeSettings::Box(const __ParticleBoxShapeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleBoxShapeSettingsInterop ScriptParticleBoxShapeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleBoxShapeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleBoxShapeSettings ScriptParticleBoxShapeSettings::FromInterop(const __ParticleBoxShapeSettingsInterop& value)
	{
		ParticleBoxShapeSettings output;
		output.Type = value.Type;
		output.Extents = value.Extents;

		return output;
	}

	__ParticleBoxShapeSettingsInterop ScriptParticleBoxShapeSettings::ToInterop(const ParticleBoxShapeSettings& value)
	{
		__ParticleBoxShapeSettingsInterop output;
		output.Type = value.Type;
		output.Extents = value.Extents;

		return output;
	}

}
