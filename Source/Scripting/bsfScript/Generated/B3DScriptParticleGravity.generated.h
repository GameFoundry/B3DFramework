//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleGravity; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleGravity : public TScriptReflectableWrapper<ParticleGravity, ScriptParticleGravity, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleGravity")

		ScriptParticleGravity(const TShared<ParticleGravity>& nativeObject);
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
