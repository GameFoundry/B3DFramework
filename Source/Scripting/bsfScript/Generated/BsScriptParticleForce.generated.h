//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleForce; }
namespace bs { struct __PARTICLE_FORCE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleForce : public TScriptReflectable<ScriptParticleForce, ParticleForce, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleForce")

		ScriptParticleForce(MonoObject* managedInstance, const SPtr<ParticleForce>& value);

		static MonoObject* Create(const SPtr<ParticleForce>& value);

	private:
		static void InternalSetOptions(ScriptParticleForce* thisPtr, __PARTICLE_FORCE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleForce* thisPtr, __PARTICLE_FORCE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_FORCE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
