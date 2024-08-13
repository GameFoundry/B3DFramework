//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "Math/BsVector3.h"

namespace bs { struct ParticleGpuSimulationSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleGpuSimulationSettings : public TScriptReflectable<ScriptParticleGpuSimulationSettings, ParticleGpuSimulationSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleGpuSimulationSettings")

		ScriptParticleGpuSimulationSettings(MonoObject* managedInstance, const SPtr<ParticleGpuSimulationSettings>& value);

		static MonoObject* Create(const SPtr<ParticleGpuSimulationSettings>& value);

	private:
		static MonoObject* InternalGetVectorField(ScriptParticleGpuSimulationSettings* self);
		static void InternalSetVectorField(ScriptParticleGpuSimulationSettings* self, MonoObject* value);
		static MonoObject* InternalGetColorOverLifetime(ScriptParticleGpuSimulationSettings* self);
		static void InternalSetColorOverLifetime(ScriptParticleGpuSimulationSettings* self, MonoObject* value);
		static MonoObject* InternalGetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* self);
		static void InternalSetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* self, MonoObject* value);
		static void InternalGetAcceleration(ScriptParticleGpuSimulationSettings* self, TVector3<float>* __output);
		static void InternalSetAcceleration(ScriptParticleGpuSimulationSettings* self, TVector3<float>* value);
		static float InternalGetDrag(ScriptParticleGpuSimulationSettings* self);
		static void InternalSetDrag(ScriptParticleGpuSimulationSettings* self, float value);
		static MonoObject* InternalGetDepthCollision(ScriptParticleGpuSimulationSettings* self);
		static void InternalSetDepthCollision(ScriptParticleGpuSimulationSettings* self, MonoObject* value);
	};
}
