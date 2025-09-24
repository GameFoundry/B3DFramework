//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace b3d { class ParticleTextureAnimation; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleTextureAnimation : public TScriptReflectableWrapper<ParticleTextureAnimation, ScriptParticleTextureAnimation, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleTextureAnimation")

		ScriptParticleTextureAnimation(const SPtr<ParticleTextureAnimation>& nativeObject);
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
