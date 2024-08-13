//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { struct __PARTICLE_ROTATION_DESCInterop; }
namespace bs { class ParticleRotation; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleRotation : public TScriptReflectable<ScriptParticleRotation, ParticleRotation, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleRotation")

		ScriptParticleRotation(MonoObject* managedInstance, const SPtr<ParticleRotation>& value);

		static MonoObject* Create(const SPtr<ParticleRotation>& value);

	private:
		static void InternalSetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_ROTATION_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
