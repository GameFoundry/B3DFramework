//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleRectangleShapeSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptTVector2.generated.h"

namespace b3d
{
	ScriptParticleRectShapeSettings::ScriptParticleRectShapeSettings()
	{ }

	MonoObject* ScriptParticleRectShapeSettings::Box(const __ParticleRectangleShapeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleRectangleShapeSettingsInterop ScriptParticleRectShapeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleRectangleShapeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleRectangleShapeSettings ScriptParticleRectShapeSettings::FromInterop(const __ParticleRectangleShapeSettingsInterop& value)
	{
		ParticleRectangleShapeSettings output;
		output.Extents = value.Extents;

		return output;
	}

	__ParticleRectangleShapeSettingsInterop ScriptParticleRectShapeSettings::ToInterop(const ParticleRectangleShapeSettings& value)
	{
		__ParticleRectangleShapeSettingsInterop output;
		output.Extents = value.Extents;

		return output;
	}

}
