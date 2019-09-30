//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsMathRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
#include "Renderer/BsCamera.h"

namespace bs
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
			BS_RTTI_MEMBER_REFL(mTransform, 2)
			BS_RTTI_MEMBER_PLAIN(mActive, 3)
			BS_RTTI_MEMBER_PLAIN(mMobility, 4)
			BS_RTTI_MEMBER_PLAIN(mProjType, 5)
			BS_RTTI_MEMBER_PLAIN(mHorzFOV, 6)
			BS_RTTI_MEMBER_PLAIN(mFarDist, 7)
			BS_RTTI_MEMBER_PLAIN(mNearDist, 8)
			BS_RTTI_MEMBER_PLAIN(mAspect, 9)
			BS_RTTI_MEMBER_PLAIN(mOrthoHeight, 10)
			BS_RTTI_MEMBER_PLAIN(mPriority, 11)
			BS_RTTI_MEMBER_PLAIN(mCustomViewMatrix, 12)
			BS_RTTI_MEMBER_PLAIN(mCustomProjMatrix, 13)
			BS_RTTI_MEMBER_PLAIN(mFrustumExtentsManuallySet, 14)
			BS_RTTI_MEMBER_PLAIN(mProjMatrixRS, 15)
			BS_RTTI_MEMBER_PLAIN(mProjMatrix, 16)
			BS_RTTI_MEMBER_PLAIN(mViewMatrix, 17)
			BS_RTTI_MEMBER_PLAIN(mLeft, 18)
			BS_RTTI_MEMBER_PLAIN(mRight, 19)
			BS_RTTI_MEMBER_PLAIN(mTop, 20)
			BS_RTTI_MEMBER_PLAIN(mBottom, 21)
			BS_RTTI_MEMBER_PLAIN(mMSAA, 22)
			BS_RTTI_MEMBER_REFLPTR(mRenderSettings, 23)
			BS_RTTI_MEMBER_PLAIN(mMain, 24)
		BS_END_RTTI_MEMBERS
		
		CameraFlags& getCameraFlags(Camera* obj)
		{
			mFlags = obj->getFlags();

			// OnDemand flag is transient and shouldn't be saved
			// (Primarily because we set it in editor on user's cameras and we don't want that to persist)
			mFlags.unset(CameraFlag::OnDemand);
			return mFlags;
		}

		void setCameraFlags(Camera* obj, CameraFlags& val) { obj->mCameraFlags = val; }
	public:
		CameraRTTI()
		{
			addPlainField("mCameraFlags", 25, &CameraRTTI::getCameraFlags, &CameraRTTI::setCameraFlags);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
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

	private:
		CameraFlags mFlags;
	};

	/** @} */
	/** @endcond */
}
