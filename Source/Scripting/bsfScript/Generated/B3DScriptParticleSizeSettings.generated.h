//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEvolver.h"
#include "../../../Foundation/Core/Particles/B3DParticleDistribution.h"
#include "../../../Foundation/Core/Particles/B3DParticleDistribution.h"

namespace b3d
{
	struct __ParticleSizeSettingsInterop
	{
		MonoObject* Size;
		MonoObject* Size3D;
		bool Use3DSize;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSizeSettings : public TScriptTypeDefinition<ScriptParticleSizeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSizeSettings")

		static MonoObject* Box(const __ParticleSizeSettingsInterop& value);
		static __ParticleSizeSettingsInterop Unbox(MonoObject* value);
		static ParticleSizeSettings FromInterop(const __ParticleSizeSettingsInterop& value);
		static __ParticleSizeSettingsInterop ToInterop(const ParticleSizeSettings& value);

	private:
		ScriptParticleSizeSettings();

	};
}
