//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Components/B3DCharacterController.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT CharacterControllerRTTI : public TRTTIType<CharacterController, Component, CharacterControllerRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(mPosition, mInformation.Position, 0)
			B3D_RTTI_MEMBER_NAMED(mContactOffset, mInformation.ContactOffset, 1)
			B3D_RTTI_MEMBER_NAMED(mStepOffset, mInformation.StepOffset, 2)
			B3D_RTTI_MEMBER_NAMED(mSlopeLimit, mInformation.SlopeLimit, 3)
			B3D_RTTI_MEMBER_NAMED(mMinMoveDistance, mInformation.MinMoveDistance, 4)
			B3D_RTTI_MEMBER_NAMED(mHeight, mInformation.Height, 5)
			B3D_RTTI_MEMBER_NAMED(mRadius, mInformation.Radius, 6)
			B3D_RTTI_MEMBER_NAMED(mUp, mInformation.Up, 7)
			B3D_RTTI_MEMBER_NAMED(mClimbingMode, mInformation.ClimbingMode, 8)
			B3D_RTTI_MEMBER_NAMED(mNonWalkableMode, mInformation.NonWalkableMode, 9)
			B3D_RTTI_MEMBER(mLayer, 10)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CharacterController";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CharacterController;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<CharacterController>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
