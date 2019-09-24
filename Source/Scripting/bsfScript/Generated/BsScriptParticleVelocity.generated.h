//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleVelocity; }
namespace bs { struct __PARTICLE_VELOCITY_DESCInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleVelocity : public TScriptReflectable<ScriptParticleVelocity, ParticleVelocity, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleVelocity")

		ScriptParticleVelocity(MonoObject* managedInstance, const SPtr<ParticleVelocity>& value);

		static MonoObject* create(const SPtr<ParticleVelocity>& value);

	private:
		static void Internal_setOptions(ScriptParticleVelocity* thisPtr, __PARTICLE_VELOCITY_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleVelocity* thisPtr, __PARTICLE_VELOCITY_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_VELOCITY_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
