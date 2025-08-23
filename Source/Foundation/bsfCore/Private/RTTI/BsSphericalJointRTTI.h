//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsMathRTTI.h"
#include "Components/BsSphericalJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT SphericalJointRTTI : public TRTTIType<SphericalJoint, Joint, SphericalJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mFlag, mInformation.Flag, 0)
			B3D_RTTI_MEMBER_NAMED(mYLimitAngle, mInformation.Limit.YLimitAngle, 1)
			B3D_RTTI_MEMBER_NAMED(mZLimitAngle, mInformation.Limit.ZLimitAngle, 2)
			B3D_RTTI_MEMBER_NAMED(mLimitContactDist, mInformation.Limit.ContactDist, 3)
			B3D_RTTI_MEMBER_NAMED(mLimitRestitution, mInformation.Limit.Restitution, 4)
			B3D_RTTI_MEMBER_NAMED(mSpringDamping, mInformation.Limit.Spring.Damping, 5)
			B3D_RTTI_MEMBER_NAMED(mSpringStiffness, mInformation.Limit.Spring.Stiffness, 6)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "SphericalJoint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SphericalJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<SphericalJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
