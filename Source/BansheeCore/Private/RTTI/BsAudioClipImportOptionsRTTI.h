//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Audio/BsAudioClipImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT AudioClipImportOptionsRTTI : public RTTIType<AudioClipImportOptions, ImportOptions, AudioClipImportOptionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mFormat, 0)
			BS_RTTI_MEMBER_PLAIN(mReadMode, 1)
			BS_RTTI_MEMBER_PLAIN(mIs3D, 2)
			BS_RTTI_MEMBER_PLAIN(mBitDepth, 3)
		BS_END_RTTI_MEMBERS
	public:
		AudioClipImportOptionsRTTI()
			:mInitMembers(this)
		{ }

		/** @copydoc RTTIType::getRTTIName */
		const String& getRTTIName() override
		{
			static String name = "AudioClipImportOptions";
			return name;
		}

		/** @copydoc RTTIType::getRTTIId */
		UINT32 getRTTIId() override
		{
			return TID_AudioClipImportOptions;
		}

		/** @copydoc RTTIType::newRTTIObject */
		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<AudioClipImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
}