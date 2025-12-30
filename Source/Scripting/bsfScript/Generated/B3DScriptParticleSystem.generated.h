//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Core/Components/B3DParticleSystem.h"
#include "../../../Foundation/Core/Components/B3DParticleSystem.h"

namespace b3d { class ParticleSystem; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSystem : public TScriptGameObjectWrapper<ParticleSystem, ScriptParticleSystem>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSystem")

		ScriptParticleSystem(const TGameObjectHandle<ParticleSystem>& nativeObject);
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
		static bool InternalTogglePreviewMode(ScriptParticleSystem* self, bool enabled);
	};
}
