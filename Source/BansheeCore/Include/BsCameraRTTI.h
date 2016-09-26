//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCamera.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT CameraRTTI : public RTTIType <Camera, IReflectable, CameraRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mViewport, 0)
			BS_RTTI_MEMBER_PLAIN(mLayers, 1)
			BS_RTTI_MEMBER_PLAIN(mPosition, 2)
			BS_RTTI_MEMBER_PLAIN(mRotation, 3)
			BS_RTTI_MEMBER_PLAIN(mProjType, 4)
			BS_RTTI_MEMBER_PLAIN(mHorzFOV, 5)
			BS_RTTI_MEMBER_PLAIN(mFarDist, 6)
			BS_RTTI_MEMBER_PLAIN(mNearDist, 7)
			BS_RTTI_MEMBER_PLAIN(mAspect, 8)
			BS_RTTI_MEMBER_PLAIN(mOrthoHeight, 9)
			BS_RTTI_MEMBER_PLAIN(mPriority, 10)
			BS_RTTI_MEMBER_PLAIN(mCustomViewMatrix, 11)
			BS_RTTI_MEMBER_PLAIN(mCustomProjMatrix, 12)
			BS_RTTI_MEMBER_PLAIN(mFrustumExtentsManuallySet, 13)
			BS_RTTI_MEMBER_PLAIN(mProjMatrixRS, 15)
			BS_RTTI_MEMBER_PLAIN(mProjMatrix, 16)
			BS_RTTI_MEMBER_PLAIN(mViewMatrix, 17)
			BS_RTTI_MEMBER_PLAIN(mLeft, 18)
			BS_RTTI_MEMBER_PLAIN(mRight, 19)
			BS_RTTI_MEMBER_PLAIN(mTop, 20)
			BS_RTTI_MEMBER_PLAIN(mBottom, 21)
			BS_RTTI_MEMBER_PLAIN(mCameraFlags, 22)
			BS_RTTI_MEMBER_PLAIN(mMSAA, 23)
			/** BS_RTTI_MEMBER_PLAIN(mPPSettings, 24) */
			BS_RTTI_MEMBER_REFLPTR(mPPSettings, 25)
		BS_END_RTTI_MEMBERS
			
	public:
		CameraRTTI()
			:mInitMembers(this)
		{ }

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component 
			// purposes (you'll need to call initialize manually).
		}

		const String& getRTTIName() override
		{
			static String name = "Camera";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Camera;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Camera::createEmpty();
		}
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(CameraFlags);

	/** @} */
	/** @endcond */
}