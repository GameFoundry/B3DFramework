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

	class B3D_CORE_EXPORT CSliderJointRTTI : public RTTIType<CSliderJoint, CJoint, CSliderJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mFlag, mDesc.Flag, 0)
			B3D_RTTI_MEMBER_NAMED(mLimitLower, mDesc.Limit.Lower, 1)
			B3D_RTTI_MEMBER_NAMED(mLimitUpper, mDesc.Limit.Upper, 2)
			B3D_RTTI_MEMBER_NAMED(mLimitContactDist, mDesc.Limit.ContactDist, 3)
			B3D_RTTI_MEMBER_NAMED(mLimitRestitution, mDesc.Limit.Restitution, 4)
			B3D_RTTI_MEMBER_NAMED(mSpringDamping, mDesc.Limit.Spring.Damping, 5)
			B3D_RTTI_MEMBER_NAMED(mSpringStiffness, mDesc.Limit.Spring.Stiffness, 6)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CSliderJoint";
			return name;
		}

		u32 GetRttiId() const override
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
} // namespace bs
