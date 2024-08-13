//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterSphereShape : public TScriptReflectable<ScriptParticleEmitterSphereShape, ParticleEmitterSphereShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmitterSphereShape")

		ScriptParticleEmitterSphereShape(MonoObject* managedInstance, const SPtr<ParticleEmitterSphereShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterSphereShape>& value);

	private:
		static void InternalSetOptions(ScriptParticleEmitterSphereShape* self, PARTICLE_SPHERE_SHAPE_DESC* options);
		static void InternalGetOptions(ScriptParticleEmitterSphereShape* self, PARTICLE_SPHERE_SHAPE_DESC* __output);
		static void InternalCreate(MonoObject* managedInstance, PARTICLE_SPHERE_SHAPE_DESC* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
