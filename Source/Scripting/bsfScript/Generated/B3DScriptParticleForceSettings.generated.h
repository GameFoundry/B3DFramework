//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace b3d
{
	struct __ParticleForceSettingsInterop
	{
		MonoObject* Force;
		bool WorldSpace;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleForceSettings : public TScriptTypeDefinition<ScriptParticleForceSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleForceSettings")

		static MonoObject* Box(const __ParticleForceSettingsInterop& value);
		static __ParticleForceSettingsInterop Unbox(MonoObject* value);
		static ParticleForceSettings FromInterop(const __ParticleForceSettingsInterop& value);
		static __ParticleForceSettingsInterop ToInterop(const ParticleForceSettings& value);

	private:
		ScriptParticleForceSettings();

	};
}
