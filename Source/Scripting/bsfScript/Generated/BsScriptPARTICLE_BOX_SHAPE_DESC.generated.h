//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __PARTICLE_BOX_SHAPE_DESCInterop
	{
		ParticleEmitterBoxType Type;
		TVector3<float> Extents;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleBoxShapeOptions : public TScriptStructWrapper<ScriptParticleBoxShapeOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleBoxShapeOptions")

		static MonoObject* Box(const __PARTICLE_BOX_SHAPE_DESCInterop& value);
		static __PARTICLE_BOX_SHAPE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_BOX_SHAPE_DESC FromInterop(const __PARTICLE_BOX_SHAPE_DESCInterop& value);
		static __PARTICLE_BOX_SHAPE_DESCInterop ToInterop(const PARTICLE_BOX_SHAPE_DESC& value);

	private:
		ScriptParticleBoxShapeOptions();

	};
}
