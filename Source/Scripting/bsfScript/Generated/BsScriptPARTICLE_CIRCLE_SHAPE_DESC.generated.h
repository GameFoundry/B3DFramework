//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	class BS_SCR_BE_EXPORT ScriptPARTICLE_CIRCLE_SHAPE_DESC : public ScriptObject<ScriptPARTICLE_CIRCLE_SHAPE_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleCircleShapeOptions")

		static MonoObject* Box(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value);
		static __PARTICLE_CIRCLE_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_CIRCLE_SHAPE_DESC FromInterop(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value);
		static __PARTICLE_CIRCLE_SHAPE_DESCInterop ToInterop(const PARTICLE_CIRCLE_SHAPE_DESC& value);

	private:
		ScriptPARTICLE_CIRCLE_SHAPE_DESC(MonoObject* managedInstance);
	};
} // namespace bs
