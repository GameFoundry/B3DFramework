//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DAudioSource.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT AudioSourceRTTI : public TRTTIType<AudioSource, Component, AudioSourceRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			// B3D_RTTI_MEMBER_REFLPTR(mInternal, 0)
			B3D_RTTI_MEMBER(mPlayOnStart, 1)
			B3D_RTTI_MEMBER(mAudioClip, 2)
			B3D_RTTI_MEMBER(mVolume, 3)
			B3D_RTTI_MEMBER(mPitch, 4)
			B3D_RTTI_MEMBER(mLoop, 5)
			B3D_RTTI_MEMBER(mPriority, 6)
			B3D_RTTI_MEMBER(mMinDistance, 7)
			B3D_RTTI_MEMBER(mAttenuation, 8)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "AudioSource";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AudioSource;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<AudioSource>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
