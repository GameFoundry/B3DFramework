//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIType.h"
#include "Audio/BsAudioSource.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT AudioSourceRTTI : public RTTIType<AudioSource, IReflectable, AudioSourceRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mTransform, 0)
			BS_RTTI_MEMBER_PLAIN(mActive, 1)
			BS_RTTI_MEMBER_PLAIN(mMobility, 2)
			BS_RTTI_MEMBER_PLAIN(mVelocity, 3)
			BS_RTTI_MEMBER_REFL(mAudioClip, 4)
			BS_RTTI_MEMBER_PLAIN(mVolume, 5)
			BS_RTTI_MEMBER_PLAIN(mPitch, 6)
			BS_RTTI_MEMBER_PLAIN(mLoop, 7)
			BS_RTTI_MEMBER_PLAIN(mPriority, 8)
			BS_RTTI_MEMBER_PLAIN(mMinDistance, 9)
			BS_RTTI_MEMBER_PLAIN(mAttenuation, 10)
		BS_END_RTTI_MEMBERS
	public:
		const String& getRTTIName() override
		{
			static String name = "AudioSource";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_AudioSource;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return AudioSource::create();
		}
	};

	/** @} */
	/** @endcond */
}
