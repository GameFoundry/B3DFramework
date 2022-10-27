//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsAABox.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"

namespace bs
{
	struct ParticleSystemSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleSystemSettings : public TScriptReflectable<ScriptParticleSystemSettings, ParticleSystemSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleSystemSettings")

		ScriptParticleSystemSettings(MonoObject* managedInstance, const SPtr<ParticleSystemSettings>& value);

		static MonoObject* Create(const SPtr<ParticleSystemSettings>& value);

	private:
		static MonoObject* InternalGetMaterial(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetMaterial(ScriptParticleSystemSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetMesh(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetMesh(ScriptParticleSystemSettings* thisPtr, MonoObject* value);
		static ParticleSimulationSpace InternalGetSimulationSpace(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetSimulationSpace(ScriptParticleSystemSettings* thisPtr, ParticleSimulationSpace value);
		static ParticleOrientation InternalGetOrientation(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetOrientation(ScriptParticleSystemSettings* thisPtr, ParticleOrientation value);
		static float InternalGetDuration(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetDuration(ScriptParticleSystemSettings* thisPtr, float value);
		static bool InternalGetIsLooping(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetIsLooping(ScriptParticleSystemSettings* thisPtr, bool value);
		static uint32_t InternalGetMaxParticles(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetMaxParticles(ScriptParticleSystemSettings* thisPtr, uint32_t value);
		static bool InternalGetGpuSimulation(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetGpuSimulation(ScriptParticleSystemSettings* thisPtr, bool value);
		static ParticleRenderMode InternalGetRenderMode(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetRenderMode(ScriptParticleSystemSettings* thisPtr, ParticleRenderMode value);
		static bool InternalGetOrientationLockY(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetOrientationLockY(ScriptParticleSystemSettings* thisPtr, bool value);
		static void InternalGetOrientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* __output);
		static void InternalSetOrientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* value);
		static ParticleSortMode InternalGetSortMode(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetSortMode(ScriptParticleSystemSettings* thisPtr, ParticleSortMode value);
		static bool InternalGetUseAutomaticSeed(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetUseAutomaticSeed(ScriptParticleSystemSettings* thisPtr, bool value);
		static uint32_t InternalGetManualSeed(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetManualSeed(ScriptParticleSystemSettings* thisPtr, uint32_t value);
		static bool InternalGetUseAutomaticBounds(ScriptParticleSystemSettings* thisPtr);
		static void InternalSetUseAutomaticBounds(ScriptParticleSystemSettings* thisPtr, bool value);
		static void InternalGetCustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* __output);
		static void InternalSetCustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* value);
	};
} // namespace bs
