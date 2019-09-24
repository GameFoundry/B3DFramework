//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleTextureAnimation; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleTextureAnimation : public TScriptReflectable<ScriptParticleTextureAnimation, ParticleTextureAnimation, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleTextureAnimation")

		ScriptParticleTextureAnimation(MonoObject* managedInstance, const SPtr<ParticleTextureAnimation>& value);

		static MonoObject* create(const SPtr<ParticleTextureAnimation>& value);

	private:
		static void Internal_setOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* options);
		static void Internal_getOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* __output);
		static void Internal_create(MonoObject* managedInstance, PARTICLE_TEXTURE_ANIMATION_DESC* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
