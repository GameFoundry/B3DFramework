//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEvolver.h"
#include "../../../Foundation/Core/Particles/B3DParticleDistribution.h"

namespace b3d
{
	struct __ParticleVelocitySettingsInterop
	{
		MonoObject* Velocity;
		bool WorldSpace;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleVelocitySettings : public TScriptTypeDefinition<ScriptParticleVelocitySettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleVelocitySettings")

		static MonoObject* Box(const __ParticleVelocitySettingsInterop& value);
		static __ParticleVelocitySettingsInterop Unbox(MonoObject* value);
		static ParticleVelocitySettings FromInterop(const __ParticleVelocitySettingsInterop& value);
		static __ParticleVelocitySettingsInterop ToInterop(const ParticleVelocitySettings& value);

	private:
		ScriptParticleVelocitySettings();

	};
}
