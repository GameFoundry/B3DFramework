//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "Math/BsVector2.h"

namespace bs
{
	struct __PARTICLE_RECT_SHAPE_DESCInterop
	{
		TVector2<float> Extents;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleRectShapeOptions : public TScriptStructWrapper<ScriptParticleRectShapeOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleRectShapeOptions")

		static MonoObject* Box(const __PARTICLE_RECT_SHAPE_DESCInterop& value);
		static __PARTICLE_RECT_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_RECT_SHAPE_DESC FromInterop(const __PARTICLE_RECT_SHAPE_DESCInterop& value);
		static __PARTICLE_RECT_SHAPE_DESCInterop ToInterop(const PARTICLE_RECT_SHAPE_DESC& value);

	private:
		ScriptParticleRectShapeOptions();

	};
}
