//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterSkinnedMeshShape; }
namespace bs { struct __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterSkinnedMeshShape : public TScriptReflectable<ScriptParticleEmitterSkinnedMeshShape, ParticleEmitterSkinnedMeshShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterSkinnedMeshShape")

		ScriptParticleEmitterSkinnedMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterSkinnedMeshShape>& value);

		static MonoObject* create(const SPtr<ParticleEmitterSkinnedMeshShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterSkinnedMeshShape* thisPtr, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleEmitterSkinnedMeshShape* thisPtr, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
