//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleOrbit; }
namespace bs { struct __PARTICLE_ORBIT_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleOrbit : public TScriptReflectableWrapper<ParticleOrbit, ScriptParticleOrbit, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleOrbit")

		ScriptParticleOrbit(const SPtr<ParticleOrbit>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleOrbit* self, __PARTICLE_ORBIT_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleOrbit* self, __PARTICLE_ORBIT_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_ORBIT_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
