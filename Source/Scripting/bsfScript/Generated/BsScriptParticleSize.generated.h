//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { struct __PARTICLE_SIZE_DESCInterop; }
namespace bs { class ParticleSize; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleSize : public TScriptReflectable<ScriptParticleSize, ParticleSize, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleSize")

		ScriptParticleSize(MonoObject* managedInstance, const SPtr<ParticleSize>& value);

		static MonoObject* create(const SPtr<ParticleSize>& value);

	private:
		static void Internal_setOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_SIZE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
