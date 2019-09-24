//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "Math/BsPlane.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleCollisions; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleCollisions : public TScriptReflectable<ScriptParticleCollisions, ParticleCollisions, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleCollisions")

		ScriptParticleCollisions(MonoObject* managedInstance, const SPtr<ParticleCollisions>& value);

		static MonoObject* create(const SPtr<ParticleCollisions>& value);

	private:
		static void Internal_setPlanes(ScriptParticleCollisions* thisPtr, MonoArray* planes);
		static MonoArray* Internal_getPlanes(ScriptParticleCollisions* thisPtr);
		static void Internal_setPlaneObjects(ScriptParticleCollisions* thisPtr, MonoArray* objects);
		static MonoArray* Internal_getPlaneObjects(ScriptParticleCollisions* thisPtr);
		static void Internal_setOptions(ScriptParticleCollisions* thisPtr, PARTICLE_COLLISIONS_DESC* options);
		static void Internal_getOptions(ScriptParticleCollisions* thisPtr, PARTICLE_COLLISIONS_DESC* __output);
		static void Internal_create(MonoObject* managedInstance, PARTICLE_COLLISIONS_DESC* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
