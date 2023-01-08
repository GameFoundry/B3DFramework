//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"

namespace bs { class CParticleSystem; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSystem : public TScriptComponent<ScriptParticleSystem, CParticleSystem>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleSystem")

		ScriptParticleSystem(MonoObject* managedInstance, const GameObjectHandle<CParticleSystem>& value);

	private:
		static void InternalSetSettings(ScriptParticleSystem* thisPtr, MonoObject* settings);
		static MonoObject* InternalGetSettings(ScriptParticleSystem* thisPtr);
		static void InternalSetGpuSimulationSettings(ScriptParticleSystem* thisPtr, MonoObject* settings);
		static MonoObject* InternalGetGpuSimulationSettings(ScriptParticleSystem* thisPtr);
		static void InternalSetEmitters(ScriptParticleSystem* thisPtr, MonoArray* emitters);
		static MonoArray* InternalGetEmitters(ScriptParticleSystem* thisPtr);
		static void InternalSetEvolvers(ScriptParticleSystem* thisPtr, MonoArray* evolvers);
		static MonoArray* InternalGetEvolvers(ScriptParticleSystem* thisPtr);
		static void InternalSetLayer(ScriptParticleSystem* thisPtr, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptParticleSystem* thisPtr);
		static bool InternalTogglePreviewModeInternal(ScriptParticleSystem* thisPtr, bool enabled);
	};
}
