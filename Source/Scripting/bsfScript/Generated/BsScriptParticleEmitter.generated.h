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

namespace bs
{
	class ParticleEmitter;
}

namespace bs
{
	struct __ParticleBurstInterop;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitter : public TScriptReflectable<ScriptParticleEmitter, ParticleEmitter>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitter")

		ScriptParticleEmitter(MonoObject* managedInstance, const SPtr<ParticleEmitter>& value);

		static MonoObject* Create(const SPtr<ParticleEmitter>& value);

	private:
		static void InternalSetShape(ScriptParticleEmitter* thisPtr, MonoObject* shape);
		static MonoObject* InternalGetShape(ScriptParticleEmitter* thisPtr);
		static void InternalSetEmissionRate(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetEmissionRate(ScriptParticleEmitter* thisPtr);
		static void InternalSetEmissionBursts(ScriptParticleEmitter* thisPtr, MonoArray* bursts);
		static MonoArray* InternalGetEmissionBursts(ScriptParticleEmitter* thisPtr);
		static void InternalSetInitialLifetime(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetInitialLifetime(ScriptParticleEmitter* thisPtr);
		static void InternalSetInitialSpeed(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetInitialSpeed(ScriptParticleEmitter* thisPtr);
		static void InternalSetInitialSize(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetInitialSize(ScriptParticleEmitter* thisPtr);
		static void InternalSetInitialSize3D(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetInitialSize3D(ScriptParticleEmitter* thisPtr);
		static void InternalSetUse3DSize(ScriptParticleEmitter* thisPtr, bool value);
		static bool InternalGetUse3DSize(ScriptParticleEmitter* thisPtr);
		static void InternalSetInitialRotation(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetInitialRotation(ScriptParticleEmitter* thisPtr);
		static void InternalSetInitialRotation3D(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetInitialRotation3D(ScriptParticleEmitter* thisPtr);
		static void InternalSetUse3DRotation(ScriptParticleEmitter* thisPtr, bool value);
		static bool InternalGetUse3DRotation(ScriptParticleEmitter* thisPtr);
		static void InternalSetInitialColor(ScriptParticleEmitter* thisPtr, MonoObject* value);
		static MonoObject* InternalGetInitialColor(ScriptParticleEmitter* thisPtr);
		static void InternalSetRandomOffset(ScriptParticleEmitter* thisPtr, float value);
		static float InternalGetRandomOffset(ScriptParticleEmitter* thisPtr);
		static void InternalSetFlipU(ScriptParticleEmitter* thisPtr, float value);
		static float InternalGetFlipU(ScriptParticleEmitter* thisPtr);
		static void InternalSetFlipV(ScriptParticleEmitter* thisPtr, float value);
		static float InternalGetFlipV(ScriptParticleEmitter* thisPtr);
		static void InternalCreate(MonoObject* managedInstance);
	};
} // namespace bs
