//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsMathRTTI.h"
#include "Components/BsCSphericalJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CSphericalJointRTTI : public RTTIType<CSphericalJoint, CJoint, CSphericalJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(mFlag, mDesc.Flag, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mYLimitAngle, mDesc.Limit.YLimitAngle, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mZLimitAngle, mDesc.Limit.ZLimitAngle, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mLimitContactDist, mDesc.Limit.ContactDist, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mLimitRestitution, mDesc.Limit.Restitution, 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mSpringDamping, mDesc.Limit.Spring.Damping, 5)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mSpringStiffness, mDesc.Limit.Spring.Stiffness, 6)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CSphericalJoint";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_CSphericalJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CSphericalJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
