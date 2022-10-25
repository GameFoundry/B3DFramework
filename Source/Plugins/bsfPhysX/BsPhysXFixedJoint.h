//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsFixedJoint.h"
#include "PxPhysics.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a FixedJoint. */
	class PhysXFixedJoint : public FixedJoint
	{
	public:
		PhysXFixedJoint(physx::PxPhysics* physx, const FIXED_JOINT_DESC& desc);
		~PhysXFixedJoint();
	};

	/** @} */
} // namespace bs
