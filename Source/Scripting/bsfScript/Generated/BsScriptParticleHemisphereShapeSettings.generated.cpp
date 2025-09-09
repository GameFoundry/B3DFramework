//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleHemisphereShapeSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptParticleHemisphereShapeSettings::ScriptParticleHemisphereShapeSettings()
	{ }

	MonoObject* ScriptParticleHemisphereShapeSettings::Box(const ParticleHemisphereShapeSettings& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	ParticleHemisphereShapeSettings ScriptParticleHemisphereShapeSettings::Unbox(MonoObject* value)
	{
		return *(ParticleHemisphereShapeSettings*)MonoUtil::Unbox(value);
	}

}
