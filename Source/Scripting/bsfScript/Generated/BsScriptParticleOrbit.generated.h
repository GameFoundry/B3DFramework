//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleOrbit; }
namespace bs { struct __PARTICLE_ORBIT_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleOrbit : public TScriptReflectable<ScriptParticleOrbit, ParticleOrbit, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleOrbit")

		ScriptParticleOrbit(MonoObject* managedInstance, const SPtr<ParticleOrbit>& value);

		static MonoObject* Create(const SPtr<ParticleOrbit>& value);

	private:
		static void InternalSetOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_ORBIT_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
