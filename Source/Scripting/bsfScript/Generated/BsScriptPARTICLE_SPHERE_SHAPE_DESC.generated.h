//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPARTICLE_SPHERE_SHAPE_DESC : public ScriptObject<ScriptPARTICLE_SPHERE_SHAPE_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleSphereShapeOptions")

		static MonoObject* Box(const PARTICLE_SPHERE_SHAPE_DESC& value);
		static PARTICLE_SPHERE_SHAPE_DESC Unbox(MonoObject* value);

	private:
		ScriptPARTICLE_SPHERE_SHAPE_DESC(MonoObject* managedInstance);

	};
}
