//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DSliderJoint.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SliderJointRTTI : public TRTTIType<SliderJoint, Joint, SliderJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mFlag, mInformation.Flags, 0)
			B3D_RTTI_MEMBER_NAMED(mLimitLower, mInformation.Limit.Lower, 1)
			B3D_RTTI_MEMBER_NAMED(mLimitUpper, mInformation.Limit.Upper, 2)
			B3D_RTTI_MEMBER_NAMED(mLimitContactDist, mInformation.Limit.ContactDist, 3)
			B3D_RTTI_MEMBER_NAMED(mLimitRestitution, mInformation.Limit.Restitution, 4)
			B3D_RTTI_MEMBER_NAMED(mSpringDamping, mInformation.Limit.Spring.Damping, 5)
			B3D_RTTI_MEMBER_NAMED(mSpringStiffness, mInformation.Limit.Spring.Stiffness, 6)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "SliderJoint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SliderJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<SliderJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
