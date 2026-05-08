//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"

namespace b3d { class ParticleEmitter; }
namespace b3d { struct __ParticleBurstInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitter : public TScriptReflectableWrapper<ParticleEmitter, ScriptParticleEmitter>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitter")

		ScriptParticleEmitter(const TShared<ParticleEmitter>& nativeObject);
		~ScriptParticleEmitter();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetShape(ScriptParticleEmitter* self, MonoObject* shape);
		static MonoObject* InternalGetShape(ScriptParticleEmitter* self);
		static void InternalSetEmissionRate(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetEmissionRate(ScriptParticleEmitter* self);
		static void InternalSetEmissionBursts(ScriptParticleEmitter* self, MonoArray* bursts);
		static MonoArray* InternalGetEmissionBursts(ScriptParticleEmitter* self);
		static void InternalSetInitialLifetime(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetInitialLifetime(ScriptParticleEmitter* self);
		static void InternalSetInitialSpeed(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetInitialSpeed(ScriptParticleEmitter* self);
		static void InternalSetInitialSize(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetInitialSize(ScriptParticleEmitter* self);
		static void InternalSetInitialSize3D(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetInitialSize3D(ScriptParticleEmitter* self);
		static void InternalSetUse3DSize(ScriptParticleEmitter* self, bool value);
		static bool InternalGetUse3DSize(ScriptParticleEmitter* self);
		static void InternalSetInitialRotation(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetInitialRotation(ScriptParticleEmitter* self);
		static void InternalSetInitialRotation3D(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetInitialRotation3D(ScriptParticleEmitter* self);
		static void InternalSetUse3DRotation(ScriptParticleEmitter* self, bool value);
		static bool InternalGetUse3DRotation(ScriptParticleEmitter* self);
		static void InternalSetInitialColor(ScriptParticleEmitter* self, MonoObject* value);
		static MonoObject* InternalGetInitialColor(ScriptParticleEmitter* self);
		static void InternalSetRandomOffset(ScriptParticleEmitter* self, float value);
		static float InternalGetRandomOffset(ScriptParticleEmitter* self);
		static void InternalSetFlipU(ScriptParticleEmitter* self, float value);
		static float InternalGetFlipU(ScriptParticleEmitter* self);
		static void InternalSetFlipV(ScriptParticleEmitter* self, float value);
		static float InternalGetFlipV(ScriptParticleEmitter* self);
		static void InternalCreate(MonoObject* scriptObject);
	};
}
