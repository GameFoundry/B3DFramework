//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "Math/BsPlane.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class ParticleCollisions;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleCollisions : public TScriptReflectable<ScriptParticleCollisions, ParticleCollisions, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleCollisions")

		ScriptParticleCollisions(MonoObject* managedInstance, const SPtr<ParticleCollisions>& value);

		static MonoObject* Create(const SPtr<ParticleCollisions>& value);

	private:
		static void InternalSetPlanes(ScriptParticleCollisions* thisPtr, MonoArray* planes);
		static MonoArray* InternalGetPlanes(ScriptParticleCollisions* thisPtr);
		static void InternalSetPlaneObjects(ScriptParticleCollisions* thisPtr, MonoArray* objects);
		static MonoArray* InternalGetPlaneObjects(ScriptParticleCollisions* thisPtr);
		static void InternalSetOptions(ScriptParticleCollisions* thisPtr, PARTICLE_COLLISIONS_DESC* options);
		static void InternalGetOptions(ScriptParticleCollisions* thisPtr, PARTICLE_COLLISIONS_DESC* __output);
		static void InternalCreate(MonoObject* managedInstance, PARTICLE_COLLISIONS_DESC* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
} // namespace bs
