//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleGravity; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleGravity : public TScriptReflectable<ScriptParticleGravity, ParticleGravity, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleGravity")

		ScriptParticleGravity(MonoObject* managedInstance, const SPtr<ParticleGravity>& value);

		static MonoObject* create(const SPtr<ParticleGravity>& value);

	private:
		static void Internal_setOptions(ScriptParticleGravity* thisPtr, PARTICLE_GRAVITY_DESC* options);
		static void Internal_getOptions(ScriptParticleGravity* thisPtr, PARTICLE_GRAVITY_DESC* __output);
		static void Internal_create(MonoObject* managedInstance, PARTICLE_GRAVITY_DESC* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
