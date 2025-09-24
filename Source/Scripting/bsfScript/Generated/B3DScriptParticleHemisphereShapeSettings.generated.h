//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleHemisphereShapeSettings : public TScriptTypeDefinition<ScriptParticleHemisphereShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleHemisphereShapeSettings")

		static MonoObject* Box(const ParticleHemisphereShapeSettings& value);
		static ParticleHemisphereShapeSettings Unbox(MonoObject* value);

	private:
		ScriptParticleHemisphereShapeSettings();

	};
}
