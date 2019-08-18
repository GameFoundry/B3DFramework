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

		float& getBreakForce(OwnerType* obj) { return obj->mDesc.breakForce; }
		void setBreakForce(OwnerType* obj, float& val) { obj->mDesc.breakForce = val; }

		float& getBreakTorque(OwnerType* obj) { return obj->mDesc.breakTorque; }
		void setBreakTorque(OwnerType* obj, float& val) { obj->mDesc.breakTorque = val; }

		bool& getEnableCollision(OwnerType* obj) { return obj->mDesc.enableCollision; }
		void setEnableCollision(OwnerType* obj, bool& val) { obj->mDesc.enableCollision = val; }

	public:
		CJointRTTI()
		{
			addPlainField("BreakForce", 6, &CJointRTTI::getBreakForce, &CJointRTTI::setBreakForce);
			addPlainField("BreakTorque", 7, &CJointRTTI::getBreakTorque, &CJointRTTI::setBreakTorque);
			addPlainField("EnableCollision", 8, &CJointRTTI::getEnableCollision, &CJointRTTI::setEnableCollision);
		}

		const String& getRTTIName() override
		{
			static String name = "CJoint";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CJoint;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}
