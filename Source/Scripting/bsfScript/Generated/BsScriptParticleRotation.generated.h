//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { struct __PARTICLE_ROTATION_DESCInterop; }
namespace bs { class ParticleRotation; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleRotation : public TScriptReflectable<ScriptParticleRotation, ParticleRotation, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleRotation")

		ScriptParticleRotation(MonoObject* managedInstance, const SPtr<ParticleRotation>& value);

		static MonoObject* create(const SPtr<ParticleRotation>& value);

	private:
		static void Internal_setOptions(ScriptParticleRotation* thisPtr, __PARTICLE_ROTATION_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleRotation* thisPtr, __PARTICLE_ROTATION_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_ROTATION_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
