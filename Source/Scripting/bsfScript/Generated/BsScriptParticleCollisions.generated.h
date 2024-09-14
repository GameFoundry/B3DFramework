//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "Math/BsPlane.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleCollisions; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleCollisions : public TScriptReflectableWrapper<ParticleCollisions, ScriptParticleCollisions, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleCollisions")

		ScriptParticleCollisions(const SPtr<ParticleCollisions>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetPlanes(ScriptParticleCollisions* self, MonoArray* planes);
		static MonoArray* InternalGetPlanes(ScriptParticleCollisions* self);
		static void InternalSetPlaneObjects(ScriptParticleCollisions* self, MonoArray* objects);
		static MonoArray* InternalGetPlaneObjects(ScriptParticleCollisions* self);
		static void InternalSetOptions(ScriptParticleCollisions* self, PARTICLE_COLLISIONS_DESC* options);
		static void InternalGetOptions(ScriptParticleCollisions* self, PARTICLE_COLLISIONS_DESC* __output);
		static void InternalCreate(MonoObject* scriptObject, PARTICLE_COLLISIONS_DESC* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
