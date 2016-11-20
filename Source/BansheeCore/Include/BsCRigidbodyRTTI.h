//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCRigidbody.h"
#include "BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CRigidbodyRTTI : public RTTIType<CRigidbody, Component, CRigidbodyRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mPositionSolverCount, 0)
			BS_RTTI_MEMBER_PLAIN(mVelocitySolverCount, 1)
			BS_RTTI_MEMBER_PLAIN(mFlags, 2)
			BS_RTTI_MEMBER_PLAIN(mCMassPosition, 3)
			BS_RTTI_MEMBER_PLAIN(mCMassRotation, 4)
			BS_RTTI_MEMBER_PLAIN(mInertiaTensor, 5)
			BS_RTTI_MEMBER_PLAIN(mMass, 6)
			BS_RTTI_MEMBER_PLAIN(mMaxAngularVelocity, 7)
			BS_RTTI_MEMBER_PLAIN(mLinearDrag, 8)
			BS_RTTI_MEMBER_PLAIN(mAngularDrag, 9)
			BS_RTTI_MEMBER_PLAIN(mSleepThreshold, 10)
			BS_RTTI_MEMBER_PLAIN(mUseGravity, 11)
			BS_RTTI_MEMBER_PLAIN(mIsKinematic, 12)
			BS_RTTI_MEMBER_PLAIN(mCollisionReportMode, 14)
		BS_END_RTTI_MEMBERS
	public:
		CRigidbodyRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CRigidbody";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CRigidbody;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CRigidbody>();
		}
	};

	/** @} */
	/** @endcond */
}