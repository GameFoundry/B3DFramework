//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { struct __PARTICLE_ROTATION_DESCInterop; }
namespace bs { class ParticleRotation; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleRotation : public TScriptReflectableWrapper<ParticleRotation, ScriptParticleRotation>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleRotation")

		ScriptParticleRotation(const SPtr<ParticleRotation>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_ROTATION_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
