//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Components/B3DJoint.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT JointRTTI : public TRTTIType<Joint, Component, JointRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mBodyA, mInformation.Bodies[0].Body, 0)
			B3D_RTTI_MEMBER_NAMED(mBodyB, mInformation.Bodies[1].Body, 1)

			B3D_RTTI_MEMBER_NAMED(mPositionA, mInformation.Bodies[0].Position, 2)
			B3D_RTTI_MEMBER_NAMED(mPositionB, mInformation.Bodies[1].Position, 3)

			B3D_RTTI_MEMBER_NAMED(mRotationA, mInformation.Bodies[0].Rotation, 4)
			B3D_RTTI_MEMBER_NAMED(mRotationB, mInformation.Bodies[1].Rotation, 5)

			B3D_RTTI_MEMBER_NAMED(mBreakForce, mInformation.BreakForce, 6)
			B3D_RTTI_MEMBER_NAMED(mBreakTorque, mInformation.BreakTorque, 7)
			B3D_RTTI_MEMBER_NAMED(mEnableCollision, mInformation.EnableCollision, 8)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "Joint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Joint;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
