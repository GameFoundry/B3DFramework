//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleForce; }
namespace bs { struct __PARTICLE_FORCE_DESCInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleForce : public TScriptReflectable<ScriptParticleForce, ParticleForce, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleForce")

		ScriptParticleForce(MonoObject* managedInstance, const SPtr<ParticleForce>& value);

		static MonoObject* create(const SPtr<ParticleForce>& value);

	private:
		static void Internal_setOptions(ScriptParticleForce* thisPtr, __PARTICLE_FORCE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleForce* thisPtr, __PARTICLE_FORCE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_FORCE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
