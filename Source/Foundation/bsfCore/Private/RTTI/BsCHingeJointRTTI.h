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

	class BS_CORE_EXPORT CHingeJointRTTI : public RTTIType<CHingeJoint, CJoint, CHingeJointRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(mFlag, mDesc.flag, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(mDriveSpeed, mDesc.drive.speed, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(mDriveForceLimit, mDesc.drive.forceLimit, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(mDriveGearRatio, mDesc.drive.gearRatio, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(mDriveFreeSpin, mDesc.drive.freeSpin, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitLower, mDesc.limit.lower, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitUpper, mDesc.limit.upper, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitContactDist, mDesc.limit.contactDist, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitRestitution, mDesc.limit.restitution, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringDamping, mDesc.limit.spring.damping, 9)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringStiffness, mDesc.limit.spring.stiffness, 10)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "CHingeJoint";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CHingeJoint;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return SceneObject::createEmptyComponent<CHingeJoint>();
		}
	};

	/** @} */
	/** @endcond */
}
