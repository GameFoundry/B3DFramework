//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "Math/BsDegree.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	struct __PARTICLE_CONE_SHAPE_DESCInterop
	{
		ParticleEmitterConeType Type;
		float Radius;
		Degree Angle;
		float Length;
		float Thickness;
		Degree Arc;
		ParticleEmissionMode Mode;
	};

	class BS_SCR_BE_EXPORT ScriptPARTICLE_CONE_SHAPE_DESC : public ScriptObject<ScriptPARTICLE_CONE_SHAPE_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleConeShapeOptions")

		static MonoObject* Box(const __PARTICLE_CONE_SHAPE_DESCInterop& value);
		static __PARTICLE_CONE_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_CONE_SHAPE_DESC FromInterop(const __PARTICLE_CONE_SHAPE_DESCInterop& value);
		static __PARTICLE_CONE_SHAPE_DESCInterop ToInterop(const PARTICLE_CONE_SHAPE_DESC& value);

	private:
		ScriptPARTICLE_CONE_SHAPE_DESC(MonoObject* managedInstance);
	};
} // namespace bs
