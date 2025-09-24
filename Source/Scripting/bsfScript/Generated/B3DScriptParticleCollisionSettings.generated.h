//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEvolver.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleCollisionsSettings : public TScriptTypeDefinition<ScriptParticleCollisionsSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleCollisionsSettings")

		static MonoObject* Box(const ParticleCollisionSettings& value);
		static ParticleCollisionSettings Unbox(MonoObject* value);

	private:
		ScriptParticleCollisionsSettings();

	};
}
