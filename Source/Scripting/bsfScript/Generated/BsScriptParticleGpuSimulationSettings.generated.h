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

namespace bs { struct ParticleGpuSimulationSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleGpuSimulationSettings : public TScriptReflectable<ScriptParticleGpuSimulationSettings, ParticleGpuSimulationSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleGpuSimulationSettings")

		ScriptParticleGpuSimulationSettings(MonoObject* managedInstance, const SPtr<ParticleGpuSimulationSettings>& value);

		static MonoObject* create(const SPtr<ParticleGpuSimulationSettings>& value);

	private:
		static MonoObject* Internal_getvectorField(ScriptParticleGpuSimulationSettings* thisPtr);
		static void Internal_setvectorField(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getcolorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr);
		static void Internal_setcolorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getsizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr);
		static void Internal_setsizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
		static void Internal_getacceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* __output);
		static void Internal_setacceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* value);
		static float Internal_getdrag(ScriptParticleGpuSimulationSettings* thisPtr);
		static void Internal_setdrag(ScriptParticleGpuSimulationSettings* thisPtr, float value);
		static MonoObject* Internal_getdepthCollision(ScriptParticleGpuSimulationSettings* thisPtr);
		static void Internal_setdepthCollision(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value);
	};
}
