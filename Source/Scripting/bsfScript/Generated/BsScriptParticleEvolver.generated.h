//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleEvolver; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEvolverWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEvolver : public TScriptReflectableWrapper<ParticleEvolver, ScriptParticleEvolver, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleEvolver")

		ScriptParticleEvolver(const SPtr<ParticleEvolver>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
