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

	class B3D_CORE_EXPORT CCharacterControllerRTTI : public RTTIType<CCharacterController, Component, CCharacterControllerRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(mPosition, mDesc.Position, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mContactOffset, mDesc.ContactOffset, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mStepOffset, mDesc.StepOffset, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mSlopeLimit, mDesc.SlopeLimit, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mMinMoveDistance, mDesc.MinMoveDistance, 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mHeight, mDesc.Height, 5)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mRadius, mDesc.Radius, 6)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mUp, mDesc.Up, 7)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mClimbingMode, mDesc.ClimbingMode, 8)
			B3D_RTTI_MEMBER_PLAIN_NAMED(mNonWalkableMode, mDesc.NonWalkableMode, 9)
			B3D_RTTI_MEMBER_PLAIN(mLayer, 10)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CCharacterController";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_CCharacterController;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<CCharacterController>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
