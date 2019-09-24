//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterSphereShape; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterSphereShape : public TScriptReflectable<ScriptParticleEmitterSphereShape, ParticleEmitterSphereShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterSphereShape")

		ScriptParticleEmitterSphereShape(MonoObject* managedInstance, const SPtr<ParticleEmitterSphereShape>& value);

		static MonoObject* create(const SPtr<ParticleEmitterSphereShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterSphereShape* thisPtr, PARTICLE_SPHERE_SHAPE_DESC* options);
		static void Internal_getOptions(ScriptParticleEmitterSphereShape* thisPtr, PARTICLE_SPHERE_SHAPE_DESC* __output);
		static void Internal_create(MonoObject* managedInstance, PARTICLE_SPHERE_SHAPE_DESC* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
