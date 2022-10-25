//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	class ParticleEmitterStaticMeshShape;
}

namespace bs
{
	struct __PARTICLE_STATIC_MESH_SHAPE_DESCInterop;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterStaticMeshShape : public TScriptReflectable<ScriptParticleEmitterStaticMeshShape, ParticleEmitterStaticMeshShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterStaticMeshShape")

		ScriptParticleEmitterStaticMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterStaticMeshShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterStaticMeshShape>& value);

	private:
		static void InternalSetOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
} // namespace bs
