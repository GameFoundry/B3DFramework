//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Utility/Math/B3DAABox.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"

namespace b3d { struct ParticleSystemSettings; }
namespace b3d { struct __TAABox_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSystemSettings : public TScriptReflectableWrapper<ParticleSystemSettings, ScriptParticleSystemSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSystemSettings")

		ScriptParticleSystemSettings(const TShared<ParticleSystemSettings>& nativeObject);
		~ScriptParticleSystemSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

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
		static void InternalGetCustomBounds(ScriptParticleSystemSettings* self, __TAABox_float_Interop* __output);
		static void InternalSetCustomBounds(ScriptParticleSystemSettings* self, __TAABox_float_Interop* value);
	};
}
