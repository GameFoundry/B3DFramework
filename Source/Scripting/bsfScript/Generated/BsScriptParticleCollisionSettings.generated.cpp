//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleCollisionSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptParticleCollisionsSettings::ScriptParticleCollisionsSettings()
	{ }

	MonoObject* ScriptParticleCollisionsSettings::Box(const ParticleCollisionSettings& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	ParticleCollisionSettings ScriptParticleCollisionsSettings::Unbox(MonoObject* value)
	{
		return *(ParticleCollisionSettings*)MonoUtil::Unbox(value);
	}

}
