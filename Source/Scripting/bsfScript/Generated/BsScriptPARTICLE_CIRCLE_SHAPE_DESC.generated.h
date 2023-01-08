//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "Math/BsDegree.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	struct __PARTICLE_CIRCLE_SHAPE_DESCInterop
	{
		float Radius;
		float Thickness;
		Degree Arc;
		ParticleEmissionMode Mode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleCircleShapeOptions : public ScriptObject<ScriptParticleCircleShapeOptions>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleCircleShapeOptions")

		static MonoObject* Box(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value);
		static __PARTICLE_CIRCLE_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_CIRCLE_SHAPE_DESC FromInterop(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value);
		static __PARTICLE_CIRCLE_SHAPE_DESCInterop ToInterop(const PARTICLE_CIRCLE_SHAPE_DESC& value);

	private:
		ScriptParticleCircleShapeOptions(MonoObject* managedInstance);

	};
}
