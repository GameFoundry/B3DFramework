//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsMathRTTI.h"
#include "Audio/BsAudioListener.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT AudioListenerRTTI : public RTTIType<AudioListener, IReflectable, AudioListenerRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mTransform, 0)
			B3D_RTTI_MEMBER_PLAIN(mActive, 1)
			B3D_RTTI_MEMBER_PLAIN(mMobility, 2)
			B3D_RTTI_MEMBER_PLAIN(mVelocity, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "AudioListener";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AudioListener;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return AudioListener::Create();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
