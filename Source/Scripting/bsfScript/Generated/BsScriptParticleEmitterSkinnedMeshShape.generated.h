//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterSkinnedMeshShape; }
namespace bs { struct __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterSkinnedMeshShape : public TScriptReflectable<ScriptParticleEmitterSkinnedMeshShape, ParticleEmitterSkinnedMeshShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmitterSkinnedMeshShape")

		ScriptParticleEmitterSkinnedMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterSkinnedMeshShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterSkinnedMeshShape>& value);

	private:
		static void InternalSetOptions(ScriptParticleEmitterSkinnedMeshShape* self, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterSkinnedMeshShape* self, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
