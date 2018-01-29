//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCDistanceJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CDistanceJointRTTI : public RTTIType<CDistanceJoint, CJoint, CDistanceJointRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(mFlag, mDesc.flag, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(mMinDistance, mDesc.minDistance, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(mMaxDistance, mDesc.maxDistance, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(mTolerance, mDesc.tolerance, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringDamping, mDesc.spring.damping, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSpringStiffness, mDesc.spring.stiffness, 5)
		BS_END_RTTI_MEMBERS

	public:
		CDistanceJointRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CDistanceJoint";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CDistanceJoint;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CDistanceJoint>();
		}
	};

	/** @} */
	/** @endcond */
}