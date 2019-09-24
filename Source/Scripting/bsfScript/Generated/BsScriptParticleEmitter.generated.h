//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs { class ParticleEmitter; }
namespace bs { struct __ParticleBurstInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitter : public TScriptReflectable<ScriptParticleEmitter, ParticleEmitter>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitter")

		ScriptParticleEmitter(MonoObject* managedInstance, const SPtr<ParticleEmitter>& value);

		static MonoObject* create(const SPtr<ParticleEmitter>& value);

	private:
		static void Internal_setShape(ScriptParticleEmitter* thisPtr, MonoObject* shape);
		static MonoObject* Internal_getShape(ScriptParticleEmitter* thisPtr);
		static void Internal_setEmissionRate(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getEmissionRate(ScriptParticleEmitter* thisPtr);
		static void Internal_setEmissionBursts(ScriptParticleEmitter* thisPtr, MonoArray* bursts);
		static MonoArray* Internal_getEmissionBursts(ScriptParticleEmitter* thisPtr);
		static void Internal_setInitialLifetime(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getInitialLifetime(ScriptParticleEmitter* thisPtr);
		static void Internal_setInitialSpeed(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getInitialSpeed(ScriptParticleEmitter* thisPtr);
		static void Internal_setInitialSize(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getInitialSize(ScriptParticleEmitter* thisPtr);
		static void Internal_setInitialSize3D(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getInitialSize3D(ScriptParticleEmitter* thisPtr);
		static void Internal_setUse3DSize(ScriptParticleEmitter* thisPtr, bool value);
		static bool Internal_getUse3DSize(ScriptParticleEmitter* thisPtr);
		static void Internal_setInitialRotation(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getInitialRotation(ScriptParticleEmitter* thisPtr);
		static void Internal_setInitialRotation3D(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getInitialRotation3D(ScriptParticleEmitter* thisPtr);
		static void Internal_setUse3DRotation(ScriptParticleEmitter* thisPtr, bool value);
		static bool Internal_getUse3DRotation(ScriptParticleEmitter* thisPtr);
		static void Internal_setInitialColor(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* Internal_getInitialColor(ScriptParticleEmitter* thisPtr);
		static void Internal_setRandomOffset(ScriptParticleEmitter* thisPtr, float value);
		static float Internal_getRandomOffset(ScriptParticleEmitter* thisPtr);
		static void Internal_setFlipU(ScriptParticleEmitter* thisPtr, float value);
		static float Internal_getFlipU(ScriptParticleEmitter* thisPtr);
		static void Internal_setFlipV(ScriptParticleEmitter* thisPtr, float value);
		static float Internal_getFlipV(ScriptParticleEmitter* thisPtr);
		static void Internal_create(MonoObject* managedInstance);
	};
}
