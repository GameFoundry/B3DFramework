//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	struct __PARTICLE_CIRCLE_SHAPE_DESCInterop;
}

namespace bs
{
	class ParticleEmitterCircleShape;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterCircleShape : public TScriptReflectable<ScriptParticleEmitterCircleShape, ParticleEmitterCircleShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmitterCircleShape")

		ScriptParticleEmitterCircleShape(MonoObject* managedInstance, const SPtr<ParticleEmitterCircleShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterCircleShape>& value);

	private:
		static void InternalSetOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_CIRCLE_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
} // namespace bs
