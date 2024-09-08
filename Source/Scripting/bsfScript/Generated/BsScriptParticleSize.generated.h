//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { struct __PARTICLE_SIZE_DESCInterop; }
namespace bs { class ParticleSize; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSize : public TScriptReflectableWrapper<ParticleSize, ScriptParticleSize, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleSize")

		ScriptParticleSize(const SPtr<ParticleSize>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleSize* self, __PARTICLE_SIZE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleSize* self, __PARTICLE_SIZE_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_SIZE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
