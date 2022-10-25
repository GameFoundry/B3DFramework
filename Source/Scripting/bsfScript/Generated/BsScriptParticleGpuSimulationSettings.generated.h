//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct ParticleGpuSimulationSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleGpuSimulationSettings : public TScriptReflectable<ScriptParticleGpuSimulationSettings, ParticleGpuSimulationSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleGpuSimulationSettings")

		ScriptParticleGpuSimulationSettings(MonoObject* managedInstance, const SPtr<ParticleGpuSimulationSettings>& value);

		static MonoObject* Create(const SPtr<ParticleGpuSimulationSettings>& value);

	private:
		static MonoObject* InternalGetVectorField(ScriptParticleGpuSimulationSettings* thisPtr);
		static void InternalSetVectorField(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetColorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr);
		static void InternalSetColorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr);
		static void InternalSetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
		static void InternalGetAcceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* __output);
		static void InternalSetAcceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* value);
		static float InternalGetDrag(ScriptParticleGpuSimulationSettings* thisPtr);
		static void InternalSetDrag(ScriptParticleGpuSimulationSettings* thisPtr, float value);
		static MonoObject* InternalGetDepthCollision(ScriptParticleGpuSimulationSettings* thisPtr);
		static void InternalSetDepthCollision(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
	};
} // namespace bs
