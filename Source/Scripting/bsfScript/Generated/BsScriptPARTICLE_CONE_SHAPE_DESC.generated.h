//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
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
		TDegree<float> Angle;
		float Length;
		float Thickness;
		TDegree<float> Arc;
		ParticleEmissionMode Mode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleConeShapeOptions : public TScriptStructWrapper<ScriptParticleConeShapeOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleConeShapeOptions")

		static MonoObject* Box(const __PARTICLE_CONE_SHAPE_DESCInterop& value);
		static __PARTICLE_CONE_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_CONE_SHAPE_DESC FromInterop(const __PARTICLE_CONE_SHAPE_DESCInterop& value);
		static __PARTICLE_CONE_SHAPE_DESCInterop ToInterop(const PARTICLE_CONE_SHAPE_DESC& value);

	private:
		ScriptParticleConeShapeOptions();

	};
}
