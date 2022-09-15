//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Components/BsCJoint.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CJointRTTI : public RTTIType<CJoint, Component, CJointRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL_NAMED(mBodyA, mBodies[0], 0)
			BS_RTTI_MEMBER_REFL_NAMED(mBodyB, mBodies[1], 1)

			BS_RTTI_MEMBER_PLAIN_NAMED(mPositionA, mPositions[0], 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(mPositionB, mPositions[1], 3)

			BS_RTTI_MEMBER_PLAIN_NAMED(mRotationA, mRotations[0], 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(mRotationB, mRotations[1], 5)
		BS_END_RTTI_MEMBERS

		float& GetBreakForce(OwnerType* obj) { return obj->mDesc.breakForce; }
		void SetBreakForce(OwnerType* obj, float& val) { obj->mDesc.breakForce = val; }

		float& GetBreakTorque(OwnerType* obj) { return obj->mDesc.breakTorque; }
		void SetBreakTorque(OwnerType* obj, float& val) { obj->mDesc.breakTorque = val; }

		bool& GetEnableCollision(OwnerType* obj) { return obj->mDesc.enableCollision; }
		void SetEnableCollision(OwnerType* obj, bool& val) { obj->mDesc.enableCollision = val; }

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

		UINT32 GetRttiId() 
		{
			return TID_CJoint;
		}

		SPtr<IReflectable> NewRttiObject() 
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}
