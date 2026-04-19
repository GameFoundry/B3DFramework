//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DD6Joint.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT D6JointDriveRTTI : public TRTTIType<D6JointDrive, IReflectable, D6JointDriveRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Stiffness, 0)
			B3D_RTTI_MEMBER(Damping, 1)
			B3D_RTTI_MEMBER(ForceLimit, 2)
			B3D_RTTI_MEMBER(Acceleration, 3)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "D6JointDrive";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_D6JointDrive;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<D6JointDrive>();
		}
	};

	class B3D_EXPORT D6JointRTTI : public TRTTIType<D6Joint, Joint, D6JointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER_NAMED(mD6JointMotion, mInformation.Motion, 0)
			B3D_RTTI_MEMBER_CONTAINER_NAMED(mD6JointDrive, mInformation.Drive, 1)

			B3D_RTTI_MEMBER_NAMED(mLimitLinearExtent, mInformation.LimitLinear.Extent, 5)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearContactDist, mInformation.LimitTwist.ContactDist, 6)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearRestitution, mInformation.LimitTwist.Restitution, 7)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearSpringDamping, mInformation.LimitTwist.Spring.Damping, 8)
			B3D_RTTI_MEMBER_NAMED(mLimitLinearSpringStiffness, mInformation.LimitTwist.Spring.Stiffness, 9)

			B3D_RTTI_MEMBER_NAMED(mLimitTwistLower, mInformation.LimitTwist.Lower, 10)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistUpper, mInformation.LimitTwist.Upper, 11)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistContactDist, mInformation.LimitTwist.ContactDist, 12)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistRestitution, mInformation.LimitTwist.Restitution, 13)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistSpringDamping, mInformation.LimitTwist.Spring.Damping, 14)
			B3D_RTTI_MEMBER_NAMED(mLimitTwistSpringStiffness, mInformation.LimitTwist.Spring.Stiffness, 15)

			B3D_RTTI_MEMBER_NAMED(mLimitSwingYLimitAngle, mInformation.LimitSwing.YLimitAngle, 16)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingZLimitAngle, mInformation.LimitSwing.ZLimitAngle, 17)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingContactDist, mInformation.LimitSwing.ContactDist, 18)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingRestitution, mInformation.LimitSwing.Restitution, 19)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingSpringDamping, mInformation.LimitSwing.Spring.Damping, 20)
			B3D_RTTI_MEMBER_NAMED(mLimitSwingSpringStiffness, mInformation.LimitSwing.Spring.Stiffness, 21)

			B3D_RTTI_MEMBER_NAMED(mDrivePosition, mInformation.DrivePosition, 22)
			B3D_RTTI_MEMBER_NAMED(mDriveRotation, mInformation.DriveRotation, 23)
			B3D_RTTI_MEMBER_NAMED(mDriveLinearVelocity, mInformation.DriveLinearVelocity, 24)
			B3D_RTTI_MEMBER_NAMED(mDriveAngularVelocity, mInformation.DriveAngularVelocity, 25)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "D6Joint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_D6Joint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<D6Joint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
