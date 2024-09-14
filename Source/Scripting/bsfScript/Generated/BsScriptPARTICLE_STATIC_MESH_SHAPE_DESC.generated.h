//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	struct __PARTICLE_STATIC_MESH_SHAPE_DESCInterop
	{
		ParticleEmitterMeshType Type;
		bool Sequential;
		MonoObject* Mesh;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleStaticMeshShapeOptions : public TScriptTypeDefinition<ScriptParticleStaticMeshShapeOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleStaticMeshShapeOptions")

		static MonoObject* Box(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value);
		static __PARTICLE_STATIC_MESH_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_STATIC_MESH_SHAPE_DESC FromInterop(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value);
		static __PARTICLE_STATIC_MESH_SHAPE_DESCInterop ToInterop(const PARTICLE_STATIC_MESH_SHAPE_DESC& value);

	private:
		ScriptParticleStaticMeshShapeOptions();

	};
}
