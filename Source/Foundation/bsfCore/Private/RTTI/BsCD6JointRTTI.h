//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCD6Joint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "Private/RTTI/BsD6JointDriveRTTI.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CD6JointRTTI : public RTTIType<CD6Joint, CJoint, CD6JointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER_NAMED(mD6JointMotion, mDesc.Motion, 0)
			B3D_RTTI_MEMBER_CONTAINER_NAMED(mD6JointDrive, mDesc.Drive, 1)

			B3D_RTTI_MEMBER_NAMED(mLimitLinearExtent, mDesc.LimitLinear.Extent, 5)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearContactDist, mDesc.LimitTwist.ContactDist, 6)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearRestitution, mDesc.LimitTwist.Restitution, 7)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearSpringDamping, mDesc.LimitTwist.Spring.Damping, 8)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearSpringStiffness, mDesc.LimitTwist.Spring.Stiffness, 9)

			B3D_RTTI_MEMBER_NAMED(mLimitTwistLower, mDesc.LimitTwist.Lower, 10)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistUpper, mDesc.LimitTwist.Upper, 11)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistContactDist, mDesc.LimitTwist.ContactDist, 12)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistRestitution, mDesc.LimitTwist.Restitution, 13)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistSpringDamping, mDesc.LimitTwist.Spring.Damping, 14)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistSpringStiffness, mDesc.LimitTwist.Spring.Stiffness, 15)

			B3D_RTTI_MEMBER_NAMED(mLimitSwingYLimitAngle, mDesc.LimitSwing.YLimitAngle, 16)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingZLimitAngle, mDesc.LimitSwing.ZLimitAngle, 17)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingContactDist, mDesc.LimitSwing.ContactDist, 18)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingRestitution, mDesc.LimitSwing.Restitution, 19)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingSpringDamping, mDesc.LimitSwing.Spring.Damping, 20)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingSpringStiffness, mDesc.LimitSwing.Spring.Stiffness, 21)

			B3D_RTTI_MEMBER_NAMED(mDrivePosition, mDesc.DrivePosition, 22)
			B3D_RTTI_MEMBER_NAMED(mDriveRotation, mDesc.DriveRotation, 23)
			B3D_RTTI_MEMBER_NAMED(mDriveLinearVelocity, mDesc.DriveLinearVelocity, 24)
			B3D_RTTI_MEMBER_NAMED(mDriveAngularVelocity, mDesc.DriveAngularVelocity, 25)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CD6Joint";
			return name;
		}

		u32 GetRttiId() const override
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
} // namespace bs
