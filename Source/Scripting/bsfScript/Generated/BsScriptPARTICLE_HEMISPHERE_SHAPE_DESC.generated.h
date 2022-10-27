//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPARTICLE_HEMISPHERE_SHAPE_DESC : public ScriptObject<ScriptPARTICLE_HEMISPHERE_SHAPE_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleHemisphereShapeOptions")

		static MonoObject* Box(const PARTICLE_HEMISPHERE_SHAPE_DESC& value);
		static PARTICLE_HEMISPHERE_SHAPE_DESC Unbox(MonoObject* value);

	private:
		ScriptPARTICLE_HEMISPHERE_SHAPE_DESC(MonoObject* managedInstance);
	};
} // namespace bs
