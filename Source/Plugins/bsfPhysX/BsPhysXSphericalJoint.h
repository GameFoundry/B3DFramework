//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsSphericalJoint.h"
#include "PxPhysics.h"
#include "extensions/PxSphericalJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a SphericalJoint. */
	class PhysXSphericalJoint : public SphericalJoint
	{
	public:
		PhysXSphericalJoint(physx::PxPhysics* physx, const SPHERICAL_JOINT_DESC& desc);
		~PhysXSphericalJoint();

		LimitConeRange GetLimit() const override;
		void SetLimit(const LimitConeRange& limit) override;
		void SetFlag(SphericalJointFlag flag, bool enabled) override;
		bool HasFlag(SphericalJointFlag flag) const override;

	private:
		/** Returns the internal PhysX representation of the spherical joint. */
		inline physx::PxSphericalJoint* GetInternal() const;
	};

	/** @} */
} // namespace bs
