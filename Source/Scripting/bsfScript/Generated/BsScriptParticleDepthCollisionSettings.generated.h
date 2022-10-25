//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"

namespace bs
{
	struct ParticleDepthCollisionSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleDepthCollisionSettings : public TScriptReflectable<ScriptParticleDepthCollisionSettings, ParticleDepthCollisionSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleDepthCollisionSettings")

		ScriptParticleDepthCollisionSettings(MonoObject* managedInstance, const SPtr<ParticleDepthCollisionSettings>& value);

		static MonoObject* Create(const SPtr<ParticleDepthCollisionSettings>& value);

	private:
		static void InternalParticleDepthCollisionSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptParticleDepthCollisionSettings* thisPtr);
		static void InternalSetEnabled(ScriptParticleDepthCollisionSettings* thisPtr, bool value);
		static float InternalGetRestitution(ScriptParticleDepthCollisionSettings* thisPtr);
		static void InternalSetRestitution(ScriptParticleDepthCollisionSettings* thisPtr, float value);
		static float InternalGetDampening(ScriptParticleDepthCollisionSettings* thisPtr);
		static void InternalSetDampening(ScriptParticleDepthCollisionSettings* thisPtr, float value);
		static float InternalGetRadiusScale(ScriptParticleDepthCollisionSettings* thisPtr);
		static void InternalSetRadiusScale(ScriptParticleDepthCollisionSettings* thisPtr, float value);
	};
} // namespace bs
