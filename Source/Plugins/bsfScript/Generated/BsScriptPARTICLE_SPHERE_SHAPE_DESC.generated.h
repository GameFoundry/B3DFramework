//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPARTICLE_SPHERE_SHAPE_DESC : public ScriptObject<ScriptPARTICLE_SPHERE_SHAPE_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleSphereShapeOptions")

		static MonoObject* box(const PARTICLE_SPHERE_SHAPE_DESC& value);
		static PARTICLE_SPHERE_SHAPE_DESC unbox(MonoObject* value);

	private:
		ScriptPARTICLE_SPHERE_SHAPE_DESC(MonoObject* managedInstance);

	};
}
