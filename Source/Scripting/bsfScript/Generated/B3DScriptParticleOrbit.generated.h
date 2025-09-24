//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Foundation/Core/Particles/B3DParticleEvolver.h"
#include "../../../Foundation/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleOrbit; }
namespace b3d { struct __ParticleOrbitSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleOrbit : public TScriptReflectableWrapper<ParticleOrbit, ScriptParticleOrbit, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleOrbit")

		ScriptParticleOrbit(const SPtr<ParticleOrbit>& nativeObject);
		~ScriptParticleOrbit();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleOrbit* self, __ParticleOrbitSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleOrbit* self, __ParticleOrbitSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleOrbitSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
