//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleVelocity; }
namespace bs { struct __PARTICLE_VELOCITY_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleVelocity : public TScriptReflectableWrapper<ParticleVelocity, ScriptParticleVelocity>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleVelocity")

		ScriptParticleVelocity(const SPtr<ParticleVelocity>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleVelocity* self, __PARTICLE_VELOCITY_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleVelocity* self, __PARTICLE_VELOCITY_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_VELOCITY_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
