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
			static String name = "CAudioSource";
			return name;
		}

		u32 GetRttiId() const override
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
