//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptParticleEmitterShape.generated.h"

namespace bs
{
	class ParticleEmitterLineShape;
	struct __PARTICLE_LINE_SHAPE_DESCInterop;

	class BS_SCR_BE_EXPORT ScriptParticleEmitterLineShape : public ScriptObject<ScriptParticleEmitterLineShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleEmitterLineShape")

		ScriptParticleEmitterLineShape(MonoObject* managedInstance, const SPtr<ParticleEmitterLineShape>& value);

		SPtr<ParticleEmitterLineShape> getInternal() const;
		static MonoObject* create(const SPtr<ParticleEmitterLineShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterLineShape* thisPtr, __PARTICLE_LINE_SHAPE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleEmitterLineShape* thisPtr, __PARTICLE_LINE_SHAPE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_LINE_SHAPE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
