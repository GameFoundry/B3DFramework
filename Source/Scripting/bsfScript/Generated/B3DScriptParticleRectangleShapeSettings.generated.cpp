//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleRectangleShapeSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "B3DScriptTVector2.generated.h"

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
