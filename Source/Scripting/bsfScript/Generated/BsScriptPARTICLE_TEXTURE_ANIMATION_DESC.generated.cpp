//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_TEXTURE_ANIMATION_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptParticleTextureAnimationOptions::ScriptParticleTextureAnimationOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleTextureAnimationOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleTextureAnimationOptions::Box(const PARTICLE_TEXTURE_ANIMATION_DESC& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	PARTICLE_TEXTURE_ANIMATION_DESC ScriptParticleTextureAnimationOptions::Unbox(MonoObject* value)
	{
		return *(PARTICLE_TEXTURE_ANIMATION_DESC*)MonoUtil::Unbox(value);
	}

}
