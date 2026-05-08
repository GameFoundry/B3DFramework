//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DCamera.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT CameraRTTI : public TRTTIType<Camera, Component, CameraRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mViewport, 0)
			B3D_RTTI_MEMBER(mLayers, 1)
			B3D_RTTI_MEMBER(mProjType, 2)
			B3D_RTTI_MEMBER(mHorzFOV, 3)
			B3D_RTTI_MEMBER(mFarDist, 4)
			B3D_RTTI_MEMBER(mNearDist, 5)
			B3D_RTTI_MEMBER(mAspect, 6)
			B3D_RTTI_MEMBER(mOrthoHeight, 7)
			B3D_RTTI_MEMBER(mPriority, 8)
			B3D_RTTI_MEMBER(mCustomViewMatrix, 9)
			B3D_RTTI_MEMBER(mCustomProjMatrix, 10)
			B3D_RTTI_MEMBER(mFrustumExtentsManuallySet, 11)
			B3D_RTTI_MEMBER(mProjMatrixRS, 12)
			B3D_RTTI_MEMBER(mProjMatrix, 13)
			B3D_RTTI_MEMBER(mViewMatrix, 14)
			B3D_RTTI_MEMBER(mLeft, 15)
			B3D_RTTI_MEMBER(mRight, 16)
			B3D_RTTI_MEMBER(mTop, 17)
			B3D_RTTI_MEMBER(mBottom, 18)
			B3D_RTTI_MEMBER(mMSAA, 19)
			B3D_RTTI_MEMBER(mRenderSettings, 20)
			B3D_RTTI_MEMBER(mMain, 21)
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

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Camera>();
		}

	private:
		CameraFlags mFlags;
	};

	/** @} */
	/** @endcond */
} // namespace b3d
