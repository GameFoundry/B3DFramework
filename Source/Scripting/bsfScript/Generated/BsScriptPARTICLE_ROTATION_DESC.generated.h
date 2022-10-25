//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
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

	class BS_SCR_BE_EXPORT ScriptPARTICLE_ROTATION_DESC : public ScriptObject<ScriptPARTICLE_ROTATION_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleRotationOptions")

		static MonoObject* Box(const __PARTICLE_ROTATION_DESCInterop& value);
		static __PARTICLE_ROTATION_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_ROTATION_DESC FromInterop(const __PARTICLE_ROTATION_DESCInterop& value);
		static __PARTICLE_ROTATION_DESCInterop ToInterop(const PARTICLE_ROTATION_DESC& value);

	private:
		ScriptPARTICLE_ROTATION_DESC(MonoObject* managedInstance);
	};
} // namespace bs
