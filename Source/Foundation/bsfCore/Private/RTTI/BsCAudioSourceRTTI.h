//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCAudioSource.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CAudioSourceRTTI : public RTTIType<CAudioSource, Component, CAudioSourceRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			// B3D_RTTI_MEMBER_REFLPTR(mInternal, 0)
			B3D_RTTI_MEMBER_PLAIN(mPlayOnStart, 1)
			B3D_RTTI_MEMBER_REFL(mAudioClip, 2)
			B3D_RTTI_MEMBER_PLAIN(mVolume, 3)
			B3D_RTTI_MEMBER_PLAIN(mPitch, 4)
			B3D_RTTI_MEMBER_PLAIN(mLoop, 5)
			B3D_RTTI_MEMBER_PLAIN(mPriority, 6)
			B3D_RTTI_MEMBER_PLAIN(mMinDistance, 7)
			B3D_RTTI_MEMBER_PLAIN(mAttenuation, 8)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CAudioSource";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_CAudioSource;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CAudioSource>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
