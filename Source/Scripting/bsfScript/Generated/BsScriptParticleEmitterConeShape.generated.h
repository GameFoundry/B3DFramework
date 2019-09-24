//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { struct __PARTICLE_CONE_SHAPE_DESCInterop; }
namespace bs { class ParticleEmitterConeShape; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterConeShape : public TScriptReflectable<ScriptParticleEmitterConeShape, ParticleEmitterConeShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterConeShape")

		ScriptParticleEmitterConeShape(MonoObject* managedInstance, const SPtr<ParticleEmitterConeShape>& value);

		static MonoObject* create(const SPtr<ParticleEmitterConeShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterConeShape* thisPtr, __PARTICLE_CONE_SHAPE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleEmitterConeShape* thisPtr, __PARTICLE_CONE_SHAPE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_CONE_SHAPE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
