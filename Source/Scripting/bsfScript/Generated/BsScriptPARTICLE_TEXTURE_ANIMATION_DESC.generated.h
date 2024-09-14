//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleTextureAnimationOptions : public TScriptTypeDefinition<ScriptParticleTextureAnimationOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleTextureAnimationOptions")

		static MonoObject* Box(const PARTICLE_TEXTURE_ANIMATION_DESC& value);
		static PARTICLE_TEXTURE_ANIMATION_DESC Unbox(MonoObject* value);

	private:
		ScriptParticleTextureAnimationOptions();

	};
}
