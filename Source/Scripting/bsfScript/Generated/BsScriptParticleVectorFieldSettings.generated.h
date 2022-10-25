//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	struct ParticleVectorFieldSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleVectorFieldSettings : public TScriptReflectable<ScriptParticleVectorFieldSettings, ParticleVectorFieldSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleVectorFieldSettings")

		ScriptParticleVectorFieldSettings(MonoObject* managedInstance, const SPtr<ParticleVectorFieldSettings>& value);

		static MonoObject* Create(const SPtr<ParticleVectorFieldSettings>& value);

	private:
		static MonoObject* InternalGetVectorField(ScriptParticleVectorFieldSettings* thisPtr);
		static void InternalSetVectorField(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value);
		static float InternalGetIntensity(ScriptParticleVectorFieldSettings* thisPtr);
		static void InternalSetIntensity(ScriptParticleVectorFieldSettings* thisPtr, float value);
		static float InternalGetTightness(ScriptParticleVectorFieldSettings* thisPtr);
		static void InternalSetTightness(ScriptParticleVectorFieldSettings* thisPtr, float value);
		static void InternalGetScale(ScriptParticleVectorFieldSettings* thisPtr, Vector3* __output);
		static void InternalSetScale(ScriptParticleVectorFieldSettings* thisPtr, Vector3* value);
		static void InternalGetOffset(ScriptParticleVectorFieldSettings* thisPtr, Vector3* __output);
		static void InternalSetOffset(ScriptParticleVectorFieldSettings* thisPtr, Vector3* value);
		static void InternalGetRotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* __output);
		static void InternalSetRotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* value);
		static MonoObject* InternalGetRotationRate(ScriptParticleVectorFieldSettings* thisPtr);
		static void InternalSetRotationRate(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value);
		static bool InternalGetTilingX(ScriptParticleVectorFieldSettings* thisPtr);
		static void InternalSetTilingX(ScriptParticleVectorFieldSettings* thisPtr, bool value);
		static bool InternalGetTilingY(ScriptParticleVectorFieldSettings* thisPtr);
		static void InternalSetTilingY(ScriptParticleVectorFieldSettings* thisPtr, bool value);
		static bool InternalGetTilingZ(ScriptParticleVectorFieldSettings* thisPtr);
		static void InternalSetTilingZ(ScriptParticleVectorFieldSettings* thisPtr, bool value);
	};
} // namespace bs
