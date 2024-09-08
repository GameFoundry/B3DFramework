//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleGravity; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleGravity : public TScriptReflectableWrapper<ParticleGravity, ScriptParticleGravity, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleGravity")

		ScriptParticleGravity(const SPtr<ParticleGravity>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleGravity* self, PARTICLE_GRAVITY_DESC* options);
		static void InternalGetOptions(ScriptParticleGravity* self, PARTICLE_GRAVITY_DESC* __output);
		static void InternalCreate(MonoObject* scriptObject, PARTICLE_GRAVITY_DESC* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
