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
			B3D_RTTI_MEMBER(mViewport, 0)
			B3D_RTTI_MEMBER(mLayers, 1)
			B3D_RTTI_MEMBER(mTransform, 2)
			B3D_RTTI_MEMBER(mActive, 3)
			B3D_RTTI_MEMBER(mMobility, 4)
			B3D_RTTI_MEMBER(mProjType, 5)
			B3D_RTTI_MEMBER(mHorzFOV, 6)
			B3D_RTTI_MEMBER(mFarDist, 7)
			B3D_RTTI_MEMBER(mNearDist, 8)
			B3D_RTTI_MEMBER(mAspect, 9)
			B3D_RTTI_MEMBER(mOrthoHeight, 10)
			B3D_RTTI_MEMBER(mPriority, 11)
			B3D_RTTI_MEMBER(mCustomViewMatrix, 12)
			B3D_RTTI_MEMBER(mCustomProjMatrix, 13)
			B3D_RTTI_MEMBER(mFrustumExtentsManuallySet, 14)
			B3D_RTTI_MEMBER(mProjMatrixRS, 15)
			B3D_RTTI_MEMBER(mProjMatrix, 16)
			B3D_RTTI_MEMBER(mViewMatrix, 17)
			B3D_RTTI_MEMBER(mLeft, 18)
			B3D_RTTI_MEMBER(mRight, 19)
			B3D_RTTI_MEMBER(mTop, 20)
			B3D_RTTI_MEMBER(mBottom, 21)
			B3D_RTTI_MEMBER(mMSAA, 22)
			B3D_RTTI_MEMBER(mRenderSettings, 23)
			B3D_RTTI_MEMBER(mMain, 24)
		B3D_RTTI_END_MEMBERS

		UPtrRTTIIterator<CameraFlags, false> GetCameraFlagsIterator(Camera& object, FrameAllocator& frameAllocator)
		{
			return CreateRTTIIterator<CameraFlags, false>(frameAllocator, object.mCameraFlags);
		}

		const CameraFlags& GetCameraFlags(Camera& object, FrameAllocator& frameAllocator, TRTTIIterator<CameraFlags, false>& iterator)
		{
			mFlags = *iterator;

			// OnDemand flag is transient and shouldn't be saved
			// (Primarily because we set it in editor on user's cameras and we don't want that to persist)
			mFlags.Unset(CameraFlag::OnDemand);
			return mFlags;
		}

		void SetCameraFlags(Camera& object, FrameAllocator& frameAllocator, TRTTIIterator<CameraFlags, false>& iterator, const CameraFlags& value)
		{
			iterator = value;
		}

	public:
		CameraRTTI()
		{
			AddField("mCameraFlags", 25, &CameraRTTI::GetCameraFlagsIterator, &CameraRTTI::GetCameraFlags, &CameraRTTI::SetCameraFlags);
		}

		const String& GetRttiName() override
		{
			static String name = "Camera";
			return name;
		}

		u32 GetRttiId() const override
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
