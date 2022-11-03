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

	class B3D_CORE_EXPORT AudioClipImportOptionsRTTI : public RTTIType<AudioClipImportOptions, ImportOptions, AudioClipImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Format, 0)
			B3D_RTTI_MEMBER_PLAIN(ReadMode, 1)
			B3D_RTTI_MEMBER_PLAIN(Is3D, 2)
			B3D_RTTI_MEMBER_PLAIN(BitDepth, 3)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "AudioClipImportOptions";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_AudioClipImportOptions;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<AudioClipImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
