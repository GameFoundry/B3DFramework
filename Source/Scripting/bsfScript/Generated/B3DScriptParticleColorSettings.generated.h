//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEvolver.h"
#include "../../../Foundation/Core/Particles/B3DParticleDistribution.h"

namespace b3d
{
	struct __ParticleColorSettingsInterop
	{
		MonoObject* Color;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleColorOptions : public TScriptTypeDefinition<ScriptParticleColorOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleColorOptions")

		static MonoObject* Box(const __ParticleColorSettingsInterop& value);
		static __ParticleColorSettingsInterop Unbox(MonoObject* value);
		static ParticleColorSettings FromInterop(const __ParticleColorSettingsInterop& value);
		static __ParticleColorSettingsInterop ToInterop(const ParticleColorSettings& value);

	private:
		ScriptParticleColorOptions();

	};
}
