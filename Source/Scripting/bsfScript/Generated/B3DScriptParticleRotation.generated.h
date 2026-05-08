//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleRotation; }
namespace b3d { struct __ParticleRotationSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleRotation : public TScriptReflectableWrapper<ParticleRotation, ScriptParticleRotation, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleRotation")

		ScriptParticleRotation(const TShared<ParticleRotation>& nativeObject);
		~ScriptParticleRotation();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleRotation* self, __ParticleRotationSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleRotation* self, __ParticleRotationSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleRotationSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
