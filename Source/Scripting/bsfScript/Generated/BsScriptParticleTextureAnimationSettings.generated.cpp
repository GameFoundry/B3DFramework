//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleTextureAnimationSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptParticleTextureAnimationSettings::ScriptParticleTextureAnimationSettings()
	{ }

	MonoObject* ScriptParticleTextureAnimationSettings::Box(const ParticleTextureAnimationSettings& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	ParticleTextureAnimationSettings ScriptParticleTextureAnimationSettings::Unbox(MonoObject* value)
	{
		return *(ParticleTextureAnimationSettings*)MonoUtil::Unbox(value);
	}

}
