//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleTextureAnimation; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleTextureAnimation : public TScriptReflectableWrapper<ParticleTextureAnimation, ScriptParticleTextureAnimation, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleTextureAnimation")

		ScriptParticleTextureAnimation(const TShared<ParticleTextureAnimation>& nativeObject);
		~ScriptParticleTextureAnimation();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleTextureAnimation* self, ParticleTextureAnimationSettings* settings);
		static void InternalGetSettings(ScriptParticleTextureAnimation* self, ParticleTextureAnimationSettings* __output);
		static void InternalCreate(MonoObject* scriptObject, ParticleTextureAnimationSettings* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
