//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace b3d { class ParticleVelocity; }
namespace b3d { struct __ParticleVelocitySettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleVelocity : public TScriptReflectableWrapper<ParticleVelocity, ScriptParticleVelocity, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleVelocity")

		ScriptParticleVelocity(const SPtr<ParticleVelocity>& nativeObject);
		~ScriptParticleVelocity();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleVelocity* self, __ParticleVelocitySettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleVelocity* self, __ParticleVelocitySettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleVelocitySettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
