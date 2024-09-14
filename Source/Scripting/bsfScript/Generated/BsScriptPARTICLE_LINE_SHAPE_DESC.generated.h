//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	struct __PARTICLE_LINE_SHAPE_DESCInterop
	{
		float Length;
		ParticleEmissionMode Mode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleLineShapeOptions : public TScriptStructWrapper<ScriptParticleLineShapeOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleLineShapeOptions")

		static MonoObject* Box(const __PARTICLE_LINE_SHAPE_DESCInterop& value);
		static __PARTICLE_LINE_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_LINE_SHAPE_DESC FromInterop(const __PARTICLE_LINE_SHAPE_DESCInterop& value);
		static __PARTICLE_LINE_SHAPE_DESCInterop ToInterop(const PARTICLE_LINE_SHAPE_DESC& value);

	private:
		ScriptParticleLineShapeOptions();

	};
}
