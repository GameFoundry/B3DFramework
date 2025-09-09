//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSphereShapeSettings : public TScriptTypeDefinition<ScriptParticleSphereShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSphereShapeSettings")

		static MonoObject* Box(const ParticleSphereShapeSettings& value);
		static ParticleSphereShapeSettings Unbox(MonoObject* value);

	private:
		ScriptParticleSphereShapeSettings();

	};
}
