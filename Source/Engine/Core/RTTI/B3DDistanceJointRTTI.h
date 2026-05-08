//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DDistanceJoint.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT DistanceJointRTTI : public TRTTIType<DistanceJoint, Joint, DistanceJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mFlag, mInformation.Flags, 0)
			B3D_RTTI_MEMBER_NAMED(mMinDistance, mInformation.MinDistance, 1)
			B3D_RTTI_MEMBER_NAMED(mMaxDistance, mInformation.MaxDistance, 2)
			B3D_RTTI_MEMBER_NAMED(mTolerance, mInformation.Tolerance, 3)
			B3D_RTTI_MEMBER_NAMED(mSpringDamping, mInformation.Spring.Damping, 4)
			B3D_RTTI_MEMBER_NAMED(mSpringStiffness, mInformation.Spring.Stiffness, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "DistanceJoint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_DistanceJoint;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<DistanceJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
