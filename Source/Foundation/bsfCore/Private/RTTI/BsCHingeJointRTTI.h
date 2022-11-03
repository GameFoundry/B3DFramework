//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Components/BsCHingeJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CHingeJointRTTI : public RTTIType<CHingeJoint, CJoint, CHingeJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(mFlag, mDesc.Flag, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mDriveSpeed, mDesc.Drive.Speed, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mDriveForceLimit, mDesc.Drive.ForceLimit, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mDriveGearRatio, mDesc.Drive.GearRatio, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mDriveFreeSpin, mDesc.Drive.FreeSpin, 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mLimitLower, mDesc.Limit.Lower, 5)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mLimitUpper, mDesc.Limit.Upper, 6)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mLimitContactDist, mDesc.Limit.ContactDist, 7)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mLimitRestitution, mDesc.Limit.Restitution, 8)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mSpringDamping, mDesc.Limit.Spring.Damping, 9)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mSpringStiffness, mDesc.Limit.Spring.Stiffness, 10)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CHingeJoint";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_CHingeJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CHingeJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
