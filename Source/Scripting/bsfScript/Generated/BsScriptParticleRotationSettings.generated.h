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
	struct __ParticleRotationSettingsInterop
	{
		MonoObject* Rotation;
		MonoObject* Rotation3D;
		bool Use3DRotation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleRotationSettings : public TScriptTypeDefinition<ScriptParticleRotationSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleRotationSettings")

		static MonoObject* Box(const __ParticleRotationSettingsInterop& value);
		static __ParticleRotationSettingsInterop Unbox(MonoObject* value);
		static ParticleRotationSettings FromInterop(const __ParticleRotationSettingsInterop& value);
		static __ParticleRotationSettingsInterop ToInterop(const ParticleRotationSettings& value);

	private:
		ScriptParticleRotationSettings();

	};
}
