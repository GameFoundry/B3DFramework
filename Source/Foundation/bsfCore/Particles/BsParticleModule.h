//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Module that in some way modified or effects a ParticleSystem. */
	class BS_CORE_EXPORT ParticleModule
	{
	protected:
		ParticleModule() = default;
		virtual ~ParticleModule() = default;

		/** Assigns the parent ParticleSystem of this module. */
		void setParent(ParticleSystem* parent) { mParent = parent; }

		ParticleSystem* mParent = nullptr;
	};

	/** @} */
}