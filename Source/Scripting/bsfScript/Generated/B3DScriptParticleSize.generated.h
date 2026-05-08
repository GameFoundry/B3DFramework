//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleSize; }
namespace b3d { struct __ParticleSizeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSize : public TScriptReflectableWrapper<ParticleSize, ScriptParticleSize, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSize")

		ScriptParticleSize(const TShared<ParticleSize>& nativeObject);
		~ScriptParticleSize();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleSize* self, __ParticleSizeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleSize* self, __ParticleSizeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleSizeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
