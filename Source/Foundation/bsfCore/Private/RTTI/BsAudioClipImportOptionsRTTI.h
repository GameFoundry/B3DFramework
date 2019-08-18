//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
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
			BS_RTTI_MEMBER_PLAIN(format, 0)
			BS_RTTI_MEMBER_PLAIN(readMode, 1)
			BS_RTTI_MEMBER_PLAIN(is3D, 2)
			BS_RTTI_MEMBER_PLAIN(bitDepth, 3)
		BS_END_RTTI_MEMBERS
	public:
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
