//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleEvolver.h"
#include "Private/Particles/BsParticleSet.h"
#include "Utility/BsTime.h"

namespace bs
{
	void ParticleDebugEvolver::evolve(Random& random, ParticleSet& set) const
	{
		ParticleSetData& particles = set.getParticles();

		UINT32 count = set.getParticleCount();
		for(UINT32 i = 0; i < count; i++)
			particles.position[i] += Vector3::UNIT_Y * gTime().getFrameDelta() * 0.1f;
	}
}
