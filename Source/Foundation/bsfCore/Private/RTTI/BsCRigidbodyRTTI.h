//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCRigidbody.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CRigidbodyRTTI : public RTTIType<CRigidbody, Component, CRigidbodyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mPositionSolverCount, 0)
			B3D_RTTI_MEMBER_PLAIN(mVelocitySolverCount, 1)
			B3D_RTTI_MEMBER_PLAIN(mFlags, 2)
			B3D_RTTI_MEMBER_PLAIN(mCMassPosition, 3)
			B3D_RTTI_MEMBER_PLAIN(mCMassRotation, 4)
			B3D_RTTI_MEMBER_PLAIN(mInertiaTensor, 5)
			B3D_RTTI_MEMBER_PLAIN(mMass, 6)
			B3D_RTTI_MEMBER_PLAIN(mMaxAngularVelocity, 7)
			B3D_RTTI_MEMBER_PLAIN(mLinearDrag, 8)
			B3D_RTTI_MEMBER_PLAIN(mAngularDrag, 9)
			B3D_RTTI_MEMBER_PLAIN(mSleepThreshold, 10)
			B3D_RTTI_MEMBER_PLAIN(mUseGravity, 11)
			B3D_RTTI_MEMBER_PLAIN(mIsKinematic, 12)
			B3D_RTTI_MEMBER_PLAIN(mCollisionReportMode, 14)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CRigidbody";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_CRigidbody;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<CRigidbody>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
