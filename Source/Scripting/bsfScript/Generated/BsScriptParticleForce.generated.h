//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace b3d { class ParticleForce; }
namespace b3d { struct __ParticleForceSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleForce : public TScriptReflectableWrapper<ParticleForce, ScriptParticleForce, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleForce")

		ScriptParticleForce(const SPtr<ParticleForce>& nativeObject);
		~ScriptParticleForce();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleForce* self, __ParticleForceSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleForce* self, __ParticleForceSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleForceSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
