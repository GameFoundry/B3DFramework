//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Components/BsCCharacterController.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CCharacterControllerRTTI : public RTTIType<CCharacterController, Component, CCharacterControllerRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(mPosition, mDesc.position, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(mContactOffset, mDesc.contactOffset, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(mStepOffset, mDesc.stepOffset, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSlopeLimit, mDesc.slopeLimit, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(mMinMoveDistance, mDesc.minMoveDistance, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(mHeight, mDesc.height, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(mRadius, mDesc.radius, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(mUp, mDesc.up, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(mClimbingMode, mDesc.climbingMode, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(mNonWalkableMode, mDesc.nonWalkableMode, 9)
			BS_RTTI_MEMBER_PLAIN(mLayer, 10)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "CCharacterController";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CCharacterController;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return SceneObject::createEmptyComponent<CCharacterController>();
		}
	};

	/** @} */
	/** @endcond */
}
