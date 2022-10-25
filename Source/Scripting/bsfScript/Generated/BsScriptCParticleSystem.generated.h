//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"

namespace bs
{
	class CParticleSystem;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCParticleSystem : public TScriptComponent<ScriptCParticleSystem, CParticleSystem>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleSystem")

		ScriptCParticleSystem(MonoObject* managedInstance, const GameObjectHandle<CParticleSystem>& value);

	private:
		static void InternalSetSettings(ScriptCParticleSystem* thisPtr, MonoObject* settings);
		static MonoObject* InternalGetSettings(ScriptCParticleSystem* thisPtr);
		static void InternalSetGpuSimulationSettings(ScriptCParticleSystem* thisPtr, MonoObject* settings);
		static MonoObject* InternalGetGpuSimulationSettings(ScriptCParticleSystem* thisPtr);
		static void InternalSetEmitters(ScriptCParticleSystem* thisPtr, MonoArray* emitters);
		static MonoArray* InternalGetEmitters(ScriptCParticleSystem* thisPtr);
		static void InternalSetEvolvers(ScriptCParticleSystem* thisPtr, MonoArray* evolvers);
		static MonoArray* InternalGetEvolvers(ScriptCParticleSystem* thisPtr);
		static void InternalSetLayer(ScriptCParticleSystem* thisPtr, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptCParticleSystem* thisPtr);
		static bool InternalTogglePreviewModeInternal(ScriptCParticleSystem* thisPtr, bool enabled);
	};
} // namespace bs
