//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __PARTICLE_BOX_SHAPE_DESCInterop
	{
		ParticleEmitterBoxType Type;
		Vector3 Extents;
	};

	class BS_SCR_BE_EXPORT ScriptPARTICLE_BOX_SHAPE_DESC : public ScriptObject<ScriptPARTICLE_BOX_SHAPE_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleBoxShapeOptions")

		static MonoObject* Box(const __PARTICLE_BOX_SHAPE_DESCInterop& value);
		static __PARTICLE_BOX_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_BOX_SHAPE_DESC FromInterop(const __PARTICLE_BOX_SHAPE_DESCInterop& value);
		static __PARTICLE_BOX_SHAPE_DESCInterop ToInterop(const PARTICLE_BOX_SHAPE_DESC& value);

	private:
		ScriptPARTICLE_BOX_SHAPE_DESC(MonoObject* managedInstance);
	};
} // namespace bs
