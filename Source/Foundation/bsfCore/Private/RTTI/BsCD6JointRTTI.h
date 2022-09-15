//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCD6Joint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CD6JointRTTI : public RTTIType<CD6Joint, CJoint, CD6JointRTTI>
	{
		D6JointMotion& GetMotion(CD6Joint* obj, UINT32 idx) { return obj->mDesc.motion[idx]; }
		void SetMotion(CD6Joint* obj, UINT32 idx, D6JointMotion& value) { obj->mDesc.motion[idx] = value; }
		UINT32 GetMotionCount(CD6Joint* obj) { return (UINT32)D6JointMotion::Count; }
		void SetMotionCount(CD6Joint* obj, UINT32 size) { /* Do nothing */ }

		float& GetDriveStiffness(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].stiffness; }
		void SetDriveStiffness(CD6Joint* obj, UINT32 idx, float& value) { obj->mDesc.drive[idx].stiffness = value; }

		float& GetDriveDamping(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].damping; }
		void SetDriveDamping(CD6Joint* obj, UINT32 idx, float& value) { obj->mDesc.drive[idx].damping = value; }

		float& GetDriveForceLimit(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].forceLimit; }
		void SetDriveForceLimit(CD6Joint* obj, UINT32 idx, float& value) { obj->mDesc.drive[idx].forceLimit = value; }

		bool& GetDriveAcceleration(CD6Joint* obj, UINT32 idx) { return obj->mDesc.drive[idx].acceleration; }
		void SetDriveAcceleration(CD6Joint* obj, UINT32 idx, bool& value) { obj->mDesc.drive[idx].acceleration = value; }

		UINT32 GetDriveCount(CD6Joint* obj) { return (UINT32)D6JointDriveType::Count; }
		void SetDriveCount(CD6Joint* obj, UINT32 size) { /* Do nothing */ }

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
		{
			AddPlainArrayField("mMotion", 0, &CD6JointRTTI::GetMotion, &CD6JointRTTI::GetMotionCount,
				&CD6JointRTTI::SetMotion, &CD6JointRTTI::SetMotionCount);
			AddPlainArrayField("mDriveStiffnes", 1, &CD6JointRTTI::GetDriveStiffness, &CD6JointRTTI::GetDriveCount,
				&CD6JointRTTI::SetDriveStiffness, &CD6JointRTTI::SetDriveCount);
			AddPlainArrayField("mDriveDamping", 2, &CD6JointRTTI::GetDriveDamping, &CD6JointRTTI::GetDriveCount,
				&CD6JointRTTI::SetDriveDamping, &CD6JointRTTI::SetDriveCount);
			AddPlainArrayField("mDriveForceLimit", 3, &CD6JointRTTI::GetDriveForceLimit, &CD6JointRTTI::GetDriveCount,
				&CD6JointRTTI::SetDriveForceLimit, &CD6JointRTTI::SetDriveCount);
			AddPlainArrayField("mDriveAcceleartion", 4, &CD6JointRTTI::GetDriveAcceleration, &CD6JointRTTI::GetDriveCount,
				&CD6JointRTTI::SetDriveAcceleration, &CD6JointRTTI::SetDriveCount);
		}

		const String& GetRttiName() override
		{
			static String name = "CD6Joint";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_CD6Joint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CD6Joint>();
		}
	};

	/** @} */
	/** @endcond */
}
