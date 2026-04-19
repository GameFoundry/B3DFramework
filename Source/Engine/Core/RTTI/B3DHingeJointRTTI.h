//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Components/B3DHingeJoint.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT HingeJointRTTI : public TRTTIType<HingeJoint, Joint, HingeJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mFlag, mInformation.Flags, 0)
			B3D_RTTI_MEMBER_NAMED(mDriveSpeed, mInformation.Drive.Speed, 1)
			B3D_RTTI_MEMBER_NAMED(mDriveForceLimit, mInformation.Drive.ForceLimit, 2)
			B3D_RTTI_MEMBER_NAMED(mDriveGearRatio, mInformation.Drive.GearRatio, 3)
			B3D_RTTI_MEMBER_NAMED(mDriveFreeSpin, mInformation.Drive.FreeSpin, 4)
			B3D_RTTI_MEMBER_NAMED(mLimitLower, mInformation.Limit.Lower, 5)
			B3D_RTTI_MEMBER_NAMED(mLimitUpper, mInformation.Limit.Upper, 6)
			B3D_RTTI_MEMBER_NAMED(mLimitContactDist, mInformation.Limit.ContactDist, 7)
			B3D_RTTI_MEMBER_NAMED(mLimitRestitution, mInformation.Limit.Restitution, 8)
			B3D_RTTI_MEMBER_NAMED(mSpringDamping, mInformation.Limit.Spring.Damping, 9)
			B3D_RTTI_MEMBER_NAMED(mSpringStiffness, mInformation.Limit.Spring.Stiffness, 10)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "HingeJoint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_HingeJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<HingeJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
