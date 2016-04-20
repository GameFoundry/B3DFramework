//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCRigidbody.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CRigidbodyRTTI : public RTTIType<CRigidbody, Component, CRigidbodyRTTI>
	{
	private:
		BS_PLAIN_MEMBER(mPositionSolverCount);
		BS_PLAIN_MEMBER(mVelocitySolverCount);
		BS_PLAIN_MEMBER(mFlags);
		BS_PLAIN_MEMBER(mCMassPosition);
		BS_PLAIN_MEMBER(mCMassRotation);
		BS_PLAIN_MEMBER(mInertiaTensor);
		BS_PLAIN_MEMBER(mMass);
		BS_PLAIN_MEMBER(mMaxAngularVelocity);
		BS_PLAIN_MEMBER(mLinearDrag);
		BS_PLAIN_MEMBER(mAngularDrag);
		BS_PLAIN_MEMBER(mSleepThreshold);
		BS_PLAIN_MEMBER(mUseGravity);
		BS_PLAIN_MEMBER(mIsKinematic);
		BS_PLAIN_MEMBER(mCollisionReportMode);

	public:
		CRigidbodyRTTI()
		{
			BS_ADD_PLAIN_FIELD(mPositionSolverCount, 0);
			BS_ADD_PLAIN_FIELD(mVelocitySolverCount, 1);
			BS_ADD_PLAIN_FIELD(mFlags, 2);
			BS_ADD_PLAIN_FIELD(mCMassPosition, 3);
			BS_ADD_PLAIN_FIELD(mCMassRotation, 4);
			BS_ADD_PLAIN_FIELD(mInertiaTensor, 5);
			BS_ADD_PLAIN_FIELD(mMass, 6);
			BS_ADD_PLAIN_FIELD(mMaxAngularVelocity, 7);
			BS_ADD_PLAIN_FIELD(mLinearDrag, 8);
			BS_ADD_PLAIN_FIELD(mAngularDrag, 9);
			BS_ADD_PLAIN_FIELD(mSleepThreshold, 10);
			BS_ADD_PLAIN_FIELD(mUseGravity, 11);
			BS_ADD_PLAIN_FIELD(mIsKinematic, 12);
			BS_ADD_PLAIN_FIELD(mCollisionReportMode, 14);
		}

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