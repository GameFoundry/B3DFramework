//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPARTICLE_TEXTURE_ANIMATION_DESC : public ScriptObject<ScriptPARTICLE_TEXTURE_ANIMATION_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleTextureAnimationOptions")

		static MonoObject* box(const PARTICLE_TEXTURE_ANIMATION_DESC& value);
		static PARTICLE_TEXTURE_ANIMATION_DESC unbox(MonoObject* value);

	private:
		ScriptPARTICLE_TEXTURE_ANIMATION_DESC(MonoObject* managedInstance);

	};
}
