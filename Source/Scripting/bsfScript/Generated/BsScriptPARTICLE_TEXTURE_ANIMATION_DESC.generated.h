//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPARTICLE_TEXTURE_ANIMATION_DESC : public ScriptObject<ScriptPARTICLE_TEXTURE_ANIMATION_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleTextureAnimationOptions")

		static MonoObject* Box(const PARTICLE_TEXTURE_ANIMATION_DESC& value);
		static PARTICLE_TEXTURE_ANIMATION_DESC Unbox(MonoObject* value);

	private:
		ScriptPARTICLE_TEXTURE_ANIMATION_DESC(MonoObject* managedInstance);

	};
}
