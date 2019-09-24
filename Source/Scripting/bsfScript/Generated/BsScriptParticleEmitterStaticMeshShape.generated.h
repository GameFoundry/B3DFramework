//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterStaticMeshShape; }
namespace bs { struct __PARTICLE_STATIC_MESH_SHAPE_DESCInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterStaticMeshShape : public TScriptReflectable<ScriptParticleEmitterStaticMeshShape, ParticleEmitterStaticMeshShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterStaticMeshShape")

		ScriptParticleEmitterStaticMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterStaticMeshShape>& value);

		static MonoObject* create(const SPtr<ParticleEmitterStaticMeshShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
