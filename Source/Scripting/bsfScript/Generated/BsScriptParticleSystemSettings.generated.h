//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsAABox.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsVector3.h"

namespace bs { struct ParticleSystemSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSystemSettings : public TScriptReflectable<ScriptParticleSystemSettings, ParticleSystemSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleSystemSettings")

		ScriptParticleSystemSettings(MonoObject* managedInstance, const SPtr<ParticleSystemSettings>& value);

		static MonoObject* Create(const SPtr<ParticleSystemSettings>& value);

	private:
		static MonoObject* InternalGetMaterial(ScriptParticleSystemSettings* self);
		static void InternalSetMaterial(ScriptParticleSystemSettings* self, MonoObject* value);
		static MonoObject* InternalGetMesh(ScriptParticleSystemSettings* self);
		static void InternalSetMesh(ScriptParticleSystemSettings* self, MonoObject* value);
		static ParticleSimulationSpace InternalGetSimulationSpace(ScriptParticleSystemSettings* self);
		static void InternalSetSimulationSpace(ScriptParticleSystemSettings* self, ParticleSimulationSpace value);
		static ParticleOrientation InternalGetOrientation(ScriptParticleSystemSettings* self);
		static void InternalSetOrientation(ScriptParticleSystemSettings* self, ParticleOrientation value);
		static float InternalGetDuration(ScriptParticleSystemSettings* self);
		static void InternalSetDuration(ScriptParticleSystemSettings* self, float value);
		static bool InternalGetIsLooping(ScriptParticleSystemSettings* self);
		static void InternalSetIsLooping(ScriptParticleSystemSettings* self, bool value);
		static uint32_t InternalGetMaxParticles(ScriptParticleSystemSettings* self);
		static void InternalSetMaxParticles(ScriptParticleSystemSettings* self, uint32_t value);
		static bool InternalGetGpuSimulation(ScriptParticleSystemSettings* self);
		static void InternalSetGpuSimulation(ScriptParticleSystemSettings* self, bool value);
		static ParticleRenderMode InternalGetRenderMode(ScriptParticleSystemSettings* self);
		static void InternalSetRenderMode(ScriptParticleSystemSettings* self, ParticleRenderMode value);
		static bool InternalGetOrientationLockY(ScriptParticleSystemSettings* self);
		static void InternalSetOrientationLockY(ScriptParticleSystemSettings* self, bool value);
		static void InternalGetOrientationPlaneNormal(ScriptParticleSystemSettings* self, TVector3<float>* __output);
		static void InternalSetOrientationPlaneNormal(ScriptParticleSystemSettings* self, TVector3<float>* value);
		static ParticleSortMode InternalGetSortMode(ScriptParticleSystemSettings* self);
		static void InternalSetSortMode(ScriptParticleSystemSettings* self, ParticleSortMode value);
		static bool InternalGetUseAutomaticSeed(ScriptParticleSystemSettings* self);
		static void InternalSetUseAutomaticSeed(ScriptParticleSystemSettings* self, bool value);
		static uint32_t InternalGetManualSeed(ScriptParticleSystemSettings* self);
		static void InternalSetManualSeed(ScriptParticleSystemSettings* self, uint32_t value);
		static bool InternalGetUseAutomaticBounds(ScriptParticleSystemSettings* self);
		static void InternalSetUseAutomaticBounds(ScriptParticleSystemSettings* self, bool value);
		static void InternalGetCustomBounds(ScriptParticleSystemSettings* self, AABox* __output);
		static void InternalSetCustomBounds(ScriptParticleSystemSettings* self, AABox* value);
	};
}
