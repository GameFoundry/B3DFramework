//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DRigidbody.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT RigidbodyRTTI : public TRTTIType<Rigidbody, Component, RigidbodyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mPositionSolverCount, 0)
			B3D_RTTI_MEMBER(mVelocitySolverCount, 1)
			B3D_RTTI_MEMBER(mFlags, 2)
			B3D_RTTI_MEMBER(mCenterOfMassPosition, 3)
			B3D_RTTI_MEMBER(mCenterOfMassRotation, 4)
			B3D_RTTI_MEMBER(mInertiaTensor, 5)
			B3D_RTTI_MEMBER(mMass, 6)
			B3D_RTTI_MEMBER(mMaxAngularVelocity, 7)
			B3D_RTTI_MEMBER(mLinearDrag, 8)
			B3D_RTTI_MEMBER(mAngularDrag, 9)
			B3D_RTTI_MEMBER(mSleepThreshold, 10)
			B3D_RTTI_MEMBER(mUseGravity, 11)
			B3D_RTTI_MEMBER(mIsKinematic, 12)
			B3D_RTTI_MEMBER(mCollisionReportMode, 14)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "Rigidbody";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Rigidbody;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<Rigidbody>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
