//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsAABox.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"

namespace bs
{
	struct ParticleSystemSettings;

	class BS_SCR_BE_EXPORT ScriptParticleSystemSettings : public ScriptObject<ScriptParticleSystemSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleSystemSettings")

		ScriptParticleSystemSettings(MonoObject* managedInstance, const SPtr<ParticleSystemSettings>& value);

		SPtr<ParticleSystemSettings> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<ParticleSystemSettings>& value);

	private:
		SPtr<ParticleSystemSettings> mInternal;

		static MonoObject* Internal_getmaterial(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setmaterial(ScriptParticleSystemSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getmesh(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setmesh(ScriptParticleSystemSettings* thisPtr, MonoObject* value);
		static bool Internal_getgpuSimulation(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setgpuSimulation(ScriptParticleSystemSettings* thisPtr, bool value);
		static ParticleSimulationSpace Internal_getsimulationSpace(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setsimulationSpace(ScriptParticleSystemSettings* thisPtr, ParticleSimulationSpace value);
		static ParticleOrientation Internal_getorientation(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setorientation(ScriptParticleSystemSettings* thisPtr, ParticleOrientation value);
		static bool Internal_getorientationLockY(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setorientationLockY(ScriptParticleSystemSettings* thisPtr, bool value);
		static void Internal_getorientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* __output);
		static void Internal_setorientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* value);
		static ParticleSortMode Internal_getsortMode(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setsortMode(ScriptParticleSystemSettings* thisPtr, ParticleSortMode value);
		static float Internal_getduration(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setduration(ScriptParticleSystemSettings* thisPtr, float value);
		static bool Internal_getisLooping(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setisLooping(ScriptParticleSystemSettings* thisPtr, bool value);
		static uint32_t Internal_getmaxParticles(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setmaxParticles(ScriptParticleSystemSettings* thisPtr, uint32_t value);
		static bool Internal_getuseAutomaticSeed(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setuseAutomaticSeed(ScriptParticleSystemSettings* thisPtr, bool value);
		static uint32_t Internal_getmanualSeed(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setmanualSeed(ScriptParticleSystemSettings* thisPtr, uint32_t value);
		static bool Internal_getuseAutomaticBounds(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setuseAutomaticBounds(ScriptParticleSystemSettings* thisPtr, bool value);
		static void Internal_getcustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* __output);
		static void Internal_setcustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* value);
		static ParticleRenderMode Internal_getrenderMode(ScriptParticleSystemSettings* thisPtr);
		static void Internal_setrenderMode(ScriptParticleSystemSettings* thisPtr, ParticleRenderMode value);
	};
}
