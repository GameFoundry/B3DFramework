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

	class B3D_CORE_EXPORT CameraRTTI : public RTTIType<Camera, IReflectable, CameraRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFLPTR(mViewport, 0)
			B3D_RTTI_MEMBER_PLAIN(mLayers, 1)
			B3D_RTTI_MEMBER_REFL(mTransform, 2)
			B3D_RTTI_MEMBER_PLAIN(mActive, 3)
			B3D_RTTI_MEMBER_PLAIN(mMobility, 4)
			B3D_RTTI_MEMBER_PLAIN(mProjType, 5)
			B3D_RTTI_MEMBER_PLAIN(mHorzFOV, 6)
			B3D_RTTI_MEMBER_PLAIN(mFarDist, 7)
			B3D_RTTI_MEMBER_PLAIN(mNearDist, 8)
			B3D_RTTI_MEMBER_PLAIN(mAspect, 9)
			B3D_RTTI_MEMBER_PLAIN(mOrthoHeight, 10)
			B3D_RTTI_MEMBER_PLAIN(mPriority, 11)
			B3D_RTTI_MEMBER_PLAIN(mCustomViewMatrix, 12)
			B3D_RTTI_MEMBER_PLAIN(mCustomProjMatrix, 13)
			B3D_RTTI_MEMBER_PLAIN(mFrustumExtentsManuallySet, 14)
			B3D_RTTI_MEMBER_PLAIN(mProjMatrixRS, 15)
			B3D_RTTI_MEMBER_PLAIN(mProjMatrix, 16)
			B3D_RTTI_MEMBER_PLAIN(mViewMatrix, 17)
			B3D_RTTI_MEMBER_PLAIN(mLeft, 18)
			B3D_RTTI_MEMBER_PLAIN(mRight, 19)
			B3D_RTTI_MEMBER_PLAIN(mTop, 20)
			B3D_RTTI_MEMBER_PLAIN(mBottom, 21)
			B3D_RTTI_MEMBER_PLAIN(mMSAA, 22)
			B3D_RTTI_MEMBER_REFLPTR(mRenderSettings, 23)
			B3D_RTTI_MEMBER_PLAIN(mMain, 24)
		B3D_RTTI_END_MEMBERS

		CameraFlags& GetCameraFlags(Camera* obj)
		{
			mFlags = obj->GetFlags();

			// OnDemand flag is transient and shouldn't be saved
			// (Primarily because we set it in editor on user's cameras and we don't want that to persist)
			mFlags.Unset(CameraFlag::OnDemand);
			return mFlags;
		}

		void SetCameraFlags(Camera* obj, CameraFlags& val) { obj->mCameraFlags = val; }

	public:
		CameraRTTI()
		{
			AddPlainField("mCameraFlags", 25, &CameraRTTI::GetCameraFlags, &CameraRTTI::SetCameraFlags);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			// Note: Since this is a CoreObject I should call Initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& GetRttiName() override
		{
			static String name = "Camera";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Camera;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Camera::CreateEmpty();
		}

	private:
		CameraFlags mFlags;
	};

	/** @} */
	/** @endcond */
} // namespace bs
