//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterStaticMeshShape; }
namespace bs { struct __PARTICLE_STATIC_MESH_SHAPE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterStaticMeshShape : public TScriptReflectableWrapper<ParticleEmitterStaticMeshShape, ScriptParticleEmitterStaticMeshShape>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleEmitterStaticMeshShape")

		ScriptParticleEmitterStaticMeshShape(const SPtr<ParticleEmitterStaticMeshShape>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleEmitterStaticMeshShape* self, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterStaticMeshShape* self, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
