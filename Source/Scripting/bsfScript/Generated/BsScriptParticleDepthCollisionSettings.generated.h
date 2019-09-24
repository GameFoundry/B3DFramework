//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"

namespace bs { struct ParticleDepthCollisionSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleDepthCollisionSettings : public TScriptReflectable<ScriptParticleDepthCollisionSettings, ParticleDepthCollisionSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleDepthCollisionSettings")

		ScriptParticleDepthCollisionSettings(MonoObject* managedInstance, const SPtr<ParticleDepthCollisionSettings>& value);

		static MonoObject* create(const SPtr<ParticleDepthCollisionSettings>& value);

	private:
		static void Internal_ParticleDepthCollisionSettings(MonoObject* managedInstance);
		static bool Internal_getenabled(ScriptParticleDepthCollisionSettings* thisPtr);
		static void Internal_setenabled(ScriptParticleDepthCollisionSettings* thisPtr, bool value);
		static float Internal_getrestitution(ScriptParticleDepthCollisionSettings* thisPtr);
		static void Internal_setrestitution(ScriptParticleDepthCollisionSettings* thisPtr, float value);
		static float Internal_getdampening(ScriptParticleDepthCollisionSettings* thisPtr);
		static void Internal_setdampening(ScriptParticleDepthCollisionSettings* thisPtr, float value);
		static float Internal_getradiusScale(ScriptParticleDepthCollisionSettings* thisPtr);
		static void Internal_setradiusScale(ScriptParticleDepthCollisionSettings* thisPtr, float value);
	};
}
