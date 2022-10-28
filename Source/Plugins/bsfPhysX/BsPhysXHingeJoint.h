//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsHingeJoint.h"
#include "PxPhysics.h"
#include "extensions/PxRevoluteJoint.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a HingeJoint. */
	class PhysXHingeJoint : public HingeJoint
	{
	public:
		PhysXHingeJoint(physx::PxPhysics* physx, const HINGE_JOINT_DESC& desc);
		~PhysXHingeJoint() override;

		Radian GetAngle() const override;
		float GetSpeed() const override;
		LimitAngularRange GetLimit() const override;
		void SetLimit(const LimitAngularRange& limit) override;
		HingeJointDrive GetDrive() const override;
		void SetDrive(const HingeJointDrive& drive) override;
		void SetFlag(HingeJointFlag flag, bool enabled) override;
		bool HasFlag(HingeJointFlag flag) const override;

	private:
		/** Returns the internal PhysX representation of the hinge (revolute) joint. */
		inline physx::PxRevoluteJoint* GetInternal() const;
	};

	/** @} */
} // namespace bs
