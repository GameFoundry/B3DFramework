//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCSphericalJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CSphericalJointRTTI : public RTTIType<CSphericalJoint, CJoint, CSphericalJointRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(mFlag, mDesc.flag, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(mYLimitAngle, mDesc.limit.yLimitAngle, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(mZLimitAngle, mDesc.limit.zLimitAngle, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitContactDist, mDesc.limit.contactDist, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(mLimitRestitution, mDesc.limit.restitution, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringDamping, mDesc.limit.spring.damping, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringStiffness, mDesc.limit.spring.stiffness, 6)
		BS_END_RTTI_MEMBERS
	public:
		CSphericalJointRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CSphericalJoint";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CSphericalJoint;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CSphericalJoint>();
		}
	};

	/** @} */
	/** @endcond */
}