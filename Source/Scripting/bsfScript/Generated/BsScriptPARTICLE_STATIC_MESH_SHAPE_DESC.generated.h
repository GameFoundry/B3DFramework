//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
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

	class BS_SCR_BE_EXPORT ScriptPARTICLE_STATIC_MESH_SHAPE_DESC : public ScriptObject<ScriptPARTICLE_STATIC_MESH_SHAPE_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleStaticMeshShapeOptions")

		static MonoObject* Box(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value);
		static __PARTICLE_STATIC_MESH_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_STATIC_MESH_SHAPE_DESC FromInterop(const __PARTICLE_STATIC_MESH_SHAPE_DESCInterop& value);
		static __PARTICLE_STATIC_MESH_SHAPE_DESCInterop ToInterop(const PARTICLE_STATIC_MESH_SHAPE_DESC& value);

	private:
		ScriptPARTICLE_STATIC_MESH_SHAPE_DESC(MonoObject* managedInstance);
	};
} // namespace bs
