//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleBoxShapeSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

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
