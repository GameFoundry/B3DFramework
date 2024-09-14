//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __PARTICLE_ROTATION_DESCInterop
	{
		MonoObject* Rotation;
		MonoObject* Rotation3D;
		bool Use3DRotation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleRotationOptions : public TScriptStructWrapper<ScriptParticleRotationOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleRotationOptions")

		static MonoObject* Box(const __PARTICLE_ROTATION_DESCInterop& value);
		static __PARTICLE_ROTATION_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_ROTATION_DESC FromInterop(const __PARTICLE_ROTATION_DESCInterop& value);
		static __PARTICLE_ROTATION_DESCInterop ToInterop(const PARTICLE_ROTATION_DESC& value);

	private:
		ScriptParticleRotationOptions();

	};
}
