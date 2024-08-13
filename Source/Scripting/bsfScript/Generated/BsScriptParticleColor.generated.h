//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleColor; }
namespace bs { struct __PARTICLE_COLOR_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleColor : public TScriptReflectable<ScriptParticleColor, ParticleColor, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleColor")

		ScriptParticleColor(MonoObject* managedInstance, const SPtr<ParticleColor>& value);

		static MonoObject* Create(const SPtr<ParticleColor>& value);

	private:
		static void InternalSetOptions(ScriptParticleColor* self, __PARTICLE_COLOR_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleColor* self, __PARTICLE_COLOR_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_COLOR_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
