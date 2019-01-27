//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptParticleEmitterShape.generated.h"

namespace bs
{
	class ParticleEmitterCircleShape;
	struct __PARTICLE_CIRCLE_SHAPE_DESCInterop;

	class BS_SCR_BE_EXPORT ScriptParticleEmitterCircleShape : public ScriptObject<ScriptParticleEmitterCircleShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleEmitterCircleShape")

		ScriptParticleEmitterCircleShape(MonoObject* managedInstance, const SPtr<ParticleEmitterCircleShape>& value);

		SPtr<ParticleEmitterCircleShape> getInternal() const;
		static MonoObject* create(const SPtr<ParticleEmitterCircleShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_CIRCLE_SHAPE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
