//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { struct __PARTICLE_SIZE_DESCInterop; }
namespace bs { class ParticleSize; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSize : public TScriptReflectable<ScriptParticleSize, ParticleSize, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleSize")

		ScriptParticleSize(MonoObject* managedInstance, const SPtr<ParticleSize>& value);

		static MonoObject* Create(const SPtr<ParticleSize>& value);

	private:
		static void InternalSetOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_SIZE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
