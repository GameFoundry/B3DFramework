//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleForce; }
namespace bs { struct __PARTICLE_FORCE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleForce : public TScriptReflectableWrapper<ParticleForce, ScriptParticleForce>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleForce")

		ScriptParticleForce(const SPtr<ParticleForce>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleForce* self, __PARTICLE_FORCE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleForce* self, __PARTICLE_FORCE_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_FORCE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
