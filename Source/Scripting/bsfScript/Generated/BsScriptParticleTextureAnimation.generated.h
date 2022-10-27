//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class ParticleTextureAnimation;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleTextureAnimation : public TScriptReflectable<ScriptParticleTextureAnimation, ParticleTextureAnimation, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleTextureAnimation")

		ScriptParticleTextureAnimation(MonoObject* managedInstance, const SPtr<ParticleTextureAnimation>& value);

		static MonoObject* Create(const SPtr<ParticleTextureAnimation>& value);

	private:
		static void InternalSetOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* options);
		static void InternalGetOptions(ScriptParticleTextureAnimation* thisPtr, PARTICLE_TEXTURE_ANIMATION_DESC* __output);
		static void InternalCreate(MonoObject* managedInstance, PARTICLE_TEXTURE_ANIMATION_DESC* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
} // namespace bs
