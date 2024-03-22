//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIType.h"
#include "Audio/BsAudioSource.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT AudioSourceRTTI : public RTTIType<AudioSource, IReflectable, AudioSourceRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mTransform, 0)
			B3D_RTTI_MEMBER_PLAIN(mActive, 1)
			B3D_RTTI_MEMBER_PLAIN(mMobility, 2)
			B3D_RTTI_MEMBER_PLAIN(mVelocity, 3)
			B3D_RTTI_MEMBER_REFL(mAudioClip, 4)
			B3D_RTTI_MEMBER_PLAIN(mVolume, 5)
			B3D_RTTI_MEMBER_PLAIN(mPitch, 6)
			B3D_RTTI_MEMBER_PLAIN(mLoop, 7)
			B3D_RTTI_MEMBER_PLAIN(mPriority, 8)
			B3D_RTTI_MEMBER_PLAIN(mMinDistance, 9)
			B3D_RTTI_MEMBER_PLAIN(mAttenuation, 10)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "AudioSource";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AudioSource;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return AudioSource::Create();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
