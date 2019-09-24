//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleOrbit; }
namespace bs { struct __PARTICLE_ORBIT_DESCInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleOrbit : public TScriptReflectable<ScriptParticleOrbit, ParticleOrbit, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleOrbit")

		ScriptParticleOrbit(MonoObject* managedInstance, const SPtr<ParticleOrbit>& value);

		static MonoObject* create(const SPtr<ParticleOrbit>& value);

	private:
		static void Internal_setOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_ORBIT_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
