//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCSliderJoint.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CSliderJointRTTI : public RTTIType<CSliderJoint, CJoint, CSliderJointRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(mFlag, mDesc.flag, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitLower, mDesc.limit.lower, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitUpper, mDesc.limit.upper, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitContactDist, mDesc.limit.contactDist, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitRestitution, mDesc.limit.restitution, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringDamping, mDesc.limit.spring.damping, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringStiffness, mDesc.limit.spring.stiffness, 6)
		BS_END_RTTI_MEMBERS
	public:
		CSliderJointRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CSliderJoint";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CSliderJoint;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CSliderJoint>();
		}
	};

	/** @} */
	/** @endcond */
}