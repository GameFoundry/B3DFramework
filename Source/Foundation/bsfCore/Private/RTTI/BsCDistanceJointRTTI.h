//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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

	class B3D_CORE_EXPORT CDistanceJointRTTI : public RTTIType<CDistanceJoint, CJoint, CDistanceJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mFlag, mDesc.Flag, 0)
			B3D_RTTI_MEMBER_NAMED(mMinDistance, mDesc.MinDistance, 1)
			B3D_RTTI_MEMBER_NAMED(mMaxDistance, mDesc.MaxDistance, 2)
			B3D_RTTI_MEMBER_NAMED(mTolerance, mDesc.Tolerance, 3)
			B3D_RTTI_MEMBER_NAMED(mSpringDamping, mDesc.Spring.Damping, 4)
			B3D_RTTI_MEMBER_NAMED(mSpringStiffness, mDesc.Spring.Stiffness, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CDistanceJoint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CDistanceJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CDistanceJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
