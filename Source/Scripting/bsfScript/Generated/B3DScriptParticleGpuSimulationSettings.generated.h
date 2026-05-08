//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"

namespace b3d { struct ParticleGpuSimulationSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleGpuSimulationSettings : public TScriptReflectableWrapper<ParticleGpuSimulationSettings, ScriptParticleGpuSimulationSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleGpuSimulationSettings")

		ScriptParticleGpuSimulationSettings(const TShared<ParticleGpuSimulationSettings>& nativeObject);
		~ScriptParticleGpuSimulationSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

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
