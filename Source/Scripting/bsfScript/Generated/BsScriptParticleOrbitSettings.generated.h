//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace b3d
{
	struct __ParticleOrbitSettingsInterop
	{
		MonoObject* Center;
		MonoObject* Velocity;
		MonoObject* Radial;
		bool WorldSpace;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleOrbitSettings : public TScriptTypeDefinition<ScriptParticleOrbitSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleOrbitSettings")

		static MonoObject* Box(const __ParticleOrbitSettingsInterop& value);
		static __ParticleOrbitSettingsInterop Unbox(MonoObject* value);
		static ParticleOrbitSettings FromInterop(const __ParticleOrbitSettingsInterop& value);
		static __ParticleOrbitSettingsInterop ToInterop(const ParticleOrbitSettings& value);

	private:
		ScriptParticleOrbitSettings();

	};
}
