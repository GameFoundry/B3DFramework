//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace b3d { class ParticleGravity; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleGravity : public TScriptReflectableWrapper<ParticleGravity, ScriptParticleGravity, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleGravity")

		ScriptParticleGravity(const SPtr<ParticleGravity>& nativeObject);
		~ScriptParticleGravity();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleGravity* self, ParticleGravitySettings* settings);
		static void InternalGetSettings(ScriptParticleGravity* self, ParticleGravitySettings* __output);
		static void InternalCreate(MonoObject* scriptObject, ParticleGravitySettings* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
