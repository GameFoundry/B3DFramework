//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleColor; }
namespace b3d { struct __ParticleColorSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleColor : public TScriptReflectableWrapper<ParticleColor, ScriptParticleColor, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleColor")

		ScriptParticleColor(const TShared<ParticleColor>& nativeObject);
		~ScriptParticleColor();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleColor* self, __ParticleColorSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleColor* self, __ParticleColorSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleColorSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
