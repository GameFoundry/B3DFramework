//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptParticleEmitterShape.generated.h"

namespace bs
{
	class ParticleEmitterStaticMeshShape;
	struct __PARTICLE_STATIC_MESH_SHAPE_DESCInterop;

	class BS_SCR_BE_EXPORT ScriptParticleEmitterStaticMeshShape : public ScriptObject<ScriptParticleEmitterStaticMeshShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleEmitterStaticMeshShape")

		ScriptParticleEmitterStaticMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterStaticMeshShape>& value);

		SPtr<ParticleEmitterStaticMeshShape> getInternal() const;
		static MonoObject* create(const SPtr<ParticleEmitterStaticMeshShape>& value);

	private:
		static void Internal_setOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
