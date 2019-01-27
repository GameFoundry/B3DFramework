//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptParticleEmitterShape.generated.h"

namespace bs
{
	class ParticleEmitterBoxShape;
	struct __PARTICLE_BOX_SHAPE_DESCInterop;

	class BS_SCR_BE_EXPORT ScriptParticleEmitterBoxShape : public ScriptObject<ScriptParticleEmitterBoxShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleEmitterBoxShape")

		ScriptParticleEmitterBoxShape(MonoObject* managedInstance, const SPtr<ParticleEmitterBoxShape>& value);

		SPtr<ParticleEmitterBoxShape> getInternal() const;
		static MonoObject* create(const SPtr<ParticleEmitterBoxShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_BOX_SHAPE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
