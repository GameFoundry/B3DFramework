//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleForce; }
namespace b3d { struct __ParticleForceSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleForce : public TScriptReflectableWrapper<ParticleForce, ScriptParticleForce, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleForce")

		ScriptParticleForce(const TShared<ParticleForce>& nativeObject);
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
