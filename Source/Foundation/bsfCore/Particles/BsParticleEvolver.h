//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Particles/BsParticleModule.h"

namespace bs
{
	class Random;
	class ParticleSet;

	/** @addtogroup Particles
	 *  @{
	 */

	/** Updates properties of all active particles in a particle system in some way. */
	class BS_CORE_EXPORT ParticleEvolver : public ParticleModule
	{
	public:
		ParticleEvolver() = default;
		virtual ~ParticleEvolver() = default;

		/** Updates properties of all particles in the @p set according to the ruleset of the evolver. */
		virtual void evolve(Random& random, ParticleSet& set) const = 0;
	};

	// TODO - Debug only
	class BS_CORE_EXPORT ParticleDebugEvolver : public ParticleEvolver
	{
	public:
		void evolve(Random& random, ParticleSet& set) const;
	};

	/** @} */
}