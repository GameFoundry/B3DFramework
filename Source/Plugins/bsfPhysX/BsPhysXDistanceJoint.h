//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsDistanceJoint.h"
#include "PxPhysics.h"
#include "extensions/PxDistanceJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a DistanceJoint */
	class PhysXDistanceJoint : public DistanceJoint
	{
	public:
		PhysXDistanceJoint(physx::PxPhysics* physx, const DISTANCE_JOINT_DESC& desc);
		~PhysXDistanceJoint() override;

		float GetDistance() const override;
		float GetMinDistance() const override;
		void SetMinDistance(float value) override;
		float GetMaxDistance() const override;
		void SetMaxDistance(float value) override;
		float GetTolerance() const override;
		void SetTolerance(float value) override;
		Spring GetSpring() const override;
		void SetSpring(const Spring& value) override;
		void SetFlag(DistanceJointFlag flag, bool enabled) override;
		bool HasFlag(DistanceJointFlag flag) const override;

	private:
		/** Returns the internal PhysX representation of the distance joint. */
		inline physx::PxDistanceJoint* GetInternal() const;
	};

	/** @} */
} // namespace bs
