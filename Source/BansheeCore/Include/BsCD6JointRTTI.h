//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCD6Joint.h"
#include "BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CD6JointRTTI : public RTTIType<CD6Joint, CJoint, CD6JointRTTI>
	{
		D6Joint::Motion& getMotion(CD6Joint* obj, UINT32 idx) { return obj->mDesc.motion[idx]; }
		void setMotion(CD6Joint* obj, UINT32 idx, D6Joint::Motion& value) { obj->mDesc.motion[idx] = value; }
		UINT32 getMotionCount(CD6Joint* obj) { return (UINT32)D6Joint::Motion::Count; }
		void setMotionCount(CD6Joint* obj, UINT32 size) { /* Do nothing */ }

		float& getDriveStiffness(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].stiffness; }
		void setDriveStiffness(CD6Joint* obj, UINT32 idx, float& value) { obj->mDesc.drive[idx].stiffness = value; }

		float& getDriveDamping(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].damping; }
		void setDriveDamping(CD6Joint* obj, UINT32 idx, float& value) { obj->mDesc.drive[idx].damping = value; }

		float& getDriveForceLimit(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].forceLimit; }
		void setDriveForceLimit(CD6Joint* obj, UINT32 idx, float& value) { obj->mDesc.drive[idx].forceLimit = value; }

		bool& getDriveAcceleration(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].acceleration; }
		void setDriveAcceleration(CD6Joint* obj, UINT32 idx, bool& value) { obj->mDesc.drive[idx].acceleration = value; }

		UINT32 getDriveCount(CD6Joint* obj) { return (UINT32)D6Joint::DriveType::Count; }
		void setDriveCount(CD6Joint* obj, UINT32 size) { /* Do nothing */ }

		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitLinearExtent, mDesc.limitLinear.extent, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitLinearContactDist, mDesc.limitTwist.contactDist, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitLinearRestitution, mDesc.limitTwist.restitution, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitLinearSpringDamping, mDesc.limitTwist.spring.damping, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitLinearSpringStiffness, mDesc.limitTwist.spring.stiffness, 9)

			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitTwistLower, mDesc.limitTwist.lower, 10)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitTwistUpper, mDesc.limitTwist.upper, 11)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitTwistContactDist, mDesc.limitTwist.contactDist, 12)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitTwistRestitution, mDesc.limitTwist.restitution, 13)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitTwistSpringDamping, mDesc.limitTwist.spring.damping, 14)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitTwistSpringStiffness, mDesc.limitTwist.spring.stiffness, 15)

			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitSwingYLimitAngle, mDesc.limitSwing.yLimitAngle, 16)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitSwingZLimitAngle, mDesc.limitSwing.zLimitAngle, 17)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitSwingContactDist, mDesc.limitSwing.contactDist, 18)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitSwingRestitution, mDesc.limitSwing.restitution, 19)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitSwingSpringDamping, mDesc.limitSwing.spring.damping, 20)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitSwingSpringStiffness, mDesc.limitSwing.spring.stiffness, 21)

			BS_RTTI_MEMBER_PLAIN_NAMED(mDrivePosition, mDesc.drivePosition, 22)
			BS_RTTI_MEMBER_PLAIN_NAMED(mDriveRotation, mDesc.driveRotation, 23)
			BS_RTTI_MEMBER_PLAIN_NAMED(mDriveLinearVelocity, mDesc.driveLinearVelocity, 24)
			BS_RTTI_MEMBER_PLAIN_NAMED(mDriveAngularVelocity, mDesc.driveAngularVelocity, 25)
		BS_END_RTTI_MEMBERS

	public:
		CD6JointRTTI()
			:mInitMembers(this)
		{
			addPlainArrayField("mMotion", 0, &CD6JointRTTI::getMotion, &CD6JointRTTI::getMotionCount, 
				&CD6JointRTTI::setMotion, &CD6JointRTTI::setMotionCount);
			addPlainArrayField("mDriveStiffnes", 1, &CD6JointRTTI::getDriveStiffness, &CD6JointRTTI::getDriveCount, 
				&CD6JointRTTI::setDriveStiffness, &CD6JointRTTI::setDriveCount);
			addPlainArrayField("mDriveDamping", 2, &CD6JointRTTI::getDriveDamping, &CD6JointRTTI::getDriveCount, 
				&CD6JointRTTI::setDriveDamping, &CD6JointRTTI::setDriveCount);
			addPlainArrayField("mDriveForceLimit", 3, &CD6JointRTTI::getDriveForceLimit, &CD6JointRTTI::getDriveCount, 
				&CD6JointRTTI::setDriveForceLimit, &CD6JointRTTI::setDriveCount);
			addPlainArrayField("mDriveAcceleartion", 4, &CD6JointRTTI::getDriveAcceleration, &CD6JointRTTI::getDriveCount, 
				&CD6JointRTTI::setDriveAcceleration, &CD6JointRTTI::setDriveCount);
		}

		const String& getRTTIName() override
		{
			static String name = "CD6Joint";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CD6Joint;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CD6Joint>();
		}
	};

	/** @} */
	/** @endcond */
}