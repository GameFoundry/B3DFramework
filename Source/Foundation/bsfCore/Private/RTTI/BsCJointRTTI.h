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
			B3D_RTTI_MEMBER_REFL_NAMED(mBodyA, mBodies[0], 0)
			B3D_RTTI_MEMBER_REFL_NAMED(mBodyB, mBodies[1], 1)

			B3D_RTTI_MEMBER_PLAIN_NAMED(mPositionA, mPositions[0], 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mPositionB, mPositions[1], 3)

			B3D_RTTI_MEMBER_PLAIN_NAMED(mRotationA, mRotations[0], 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mRotationB, mRotations[1], 5)
		B3D_RTTI_END_MEMBERS

		float& GetBreakForce(OwnerType* obj) { return obj->mDesc.BreakForce; }

		void SetBreakForce(OwnerType* obj, float& val) { obj->mDesc.BreakForce = val; }

		float& GetBreakTorque(OwnerType* obj) { return obj->mDesc.BreakTorque; }

		void SetBreakTorque(OwnerType* obj, float& val) { obj->mDesc.BreakTorque = val; }

		bool& GetEnableCollision(OwnerType* obj) { return obj->mDesc.EnableCollision; }

		void SetEnableCollision(OwnerType* obj, bool& val) { obj->mDesc.EnableCollision = val; }

	public:
		CJointRTTI()
		{
			AddPlainField("BreakForce", 6, &CJointRTTI::GetBreakForce, &CJointRTTI::SetBreakForce);
			AddPlainField("BreakTorque", 7, &CJointRTTI::GetBreakTorque, &CJointRTTI::SetBreakTorque);
			AddPlainField("EnableCollision", 8, &CJointRTTI::GetEnableCollision, &CJointRTTI::SetEnableCollision);
		}

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
