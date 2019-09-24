//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterHemisphereShape; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterHemisphereShape : public TScriptReflectable<ScriptParticleEmitterHemisphereShape, ParticleEmitterHemisphereShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterHemisphereShape")

		ScriptParticleEmitterHemisphereShape(MonoObject* managedInstance, const SPtr<ParticleEmitterHemisphereShape>& value);

		static MonoObject* create(const SPtr<ParticleEmitterHemisphereShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterHemisphereShape* thisPtr, PARTICLE_HEMISPHERE_SHAPE_DESC* options);
		static void Internal_getOptions(ScriptParticleEmitterHemisphereShape* thisPtr, PARTICLE_HEMISPHERE_SHAPE_DESC* __output);
		static void Internal_create(MonoObject* managedInstance, PARTICLE_HEMISPHERE_SHAPE_DESC* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
