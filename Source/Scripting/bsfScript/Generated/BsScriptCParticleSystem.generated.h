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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSystem : public TScriptGameObjectWrapper<CParticleSystem, ScriptParticleSystem>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSystem")

		ScriptParticleSystem(const GameObjectHandle<CParticleSystem>& nativeObject);
		~ScriptParticleSystem();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleSystem* self, MonoObject* settings);
		static MonoObject* InternalGetSettings(ScriptParticleSystem* self);
		static void InternalSetGpuSimulationSettings(ScriptParticleSystem* self, MonoObject* settings);
		static MonoObject* InternalGetGpuSimulationSettings(ScriptParticleSystem* self);
		static void InternalSetEmitters(ScriptParticleSystem* self, MonoArray* emitters);
		static MonoArray* InternalGetEmitters(ScriptParticleSystem* self);
		static void InternalSetEvolvers(ScriptParticleSystem* self, MonoArray* evolvers);
		static MonoArray* InternalGetEvolvers(ScriptParticleSystem* self);
		static void InternalSetLayer(ScriptParticleSystem* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptParticleSystem* self);
		static bool InternalTogglePreviewModeInternal(ScriptParticleSystem* self, bool enabled);
	};
}
