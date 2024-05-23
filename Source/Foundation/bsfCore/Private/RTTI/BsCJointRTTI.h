//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Components/BsCJoint.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CJointRTTI : public RTTIType<CJoint, Component, CJointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mBodyA, mBodies[0], 0)
			B3D_RTTI_MEMBER_NAMED(mBodyB, mBodies[1], 1)

			B3D_RTTI_MEMBER_NAMED(mPositionA, mPositions[0], 2)
			B3D_RTTI_MEMBER_NAMED(mPositionB, mPositions[1], 3)

			B3D_RTTI_MEMBER_NAMED(mRotationA, mRotations[0], 4)
			B3D_RTTI_MEMBER_NAMED(mRotationB, mRotations[1], 5)

			B3D_RTTI_MEMBER_NAMED(mBreakForce, mDesc.BreakForce, 6)
			B3D_RTTI_MEMBER_NAMED(mBreakTorque, mDesc.BreakTorque, 7)
			B3D_RTTI_MEMBER_NAMED(mEnableCollision, mDesc.EnableCollision, 8)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "CJoint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CJoint;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
