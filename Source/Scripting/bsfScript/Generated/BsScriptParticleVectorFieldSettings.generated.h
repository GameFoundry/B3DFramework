//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "Math/BsQuaternion.h"

namespace bs { struct ParticleVectorFieldSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleVectorFieldSettings : public TScriptReflectable<ScriptParticleVectorFieldSettings, ParticleVectorFieldSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleVectorFieldSettings")

		ScriptParticleVectorFieldSettings(MonoObject* managedInstance, const SPtr<ParticleVectorFieldSettings>& value);

		static MonoObject* create(const SPtr<ParticleVectorFieldSettings>& value);

	private:
		static MonoObject* Internal_getvectorField(ScriptParticleVectorFieldSettings* thisPtr);
		static void Internal_setvectorField(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value);
		static float Internal_getintensity(ScriptParticleVectorFieldSettings* thisPtr);
		static void Internal_setintensity(ScriptParticleVectorFieldSettings* thisPtr, float value);
		static float Internal_gettightness(ScriptParticleVectorFieldSettings* thisPtr);
		static void Internal_settightness(ScriptParticleVectorFieldSettings* thisPtr, float value);
		static void Internal_getscale(ScriptParticleVectorFieldSettings* thisPtr, Vector3* __output);
		static void Internal_setscale(ScriptParticleVectorFieldSettings* thisPtr, Vector3* value);
		static void Internal_getoffset(ScriptParticleVectorFieldSettings* thisPtr, Vector3* __output);
		static void Internal_setoffset(ScriptParticleVectorFieldSettings* thisPtr, Vector3* value);
		static void Internal_getrotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* __output);
		static void Internal_setrotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* value);
		static MonoObject* Internal_getrotationRate(ScriptParticleVectorFieldSettings* thisPtr);
		static void Internal_setrotationRate(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value);
		static bool Internal_gettilingX(ScriptParticleVectorFieldSettings* thisPtr);
		static void Internal_settilingX(ScriptParticleVectorFieldSettings* thisPtr, bool value);
		static bool Internal_gettilingY(ScriptParticleVectorFieldSettings* thisPtr);
		static void Internal_settilingY(ScriptParticleVectorFieldSettings* thisPtr, bool value);
		static bool Internal_gettilingZ(ScriptParticleVectorFieldSettings* thisPtr);
		static void Internal_settilingZ(ScriptParticleVectorFieldSettings* thisPtr, bool value);
	};
}
