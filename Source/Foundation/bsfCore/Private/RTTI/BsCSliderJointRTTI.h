//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCSliderJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
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
		const String& GetRttiName() override
		{
			static String name = "CSliderJoint";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_CSliderJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CSliderJoint>();
		}
	};

	/** @} */
	/** @endcond */
}
