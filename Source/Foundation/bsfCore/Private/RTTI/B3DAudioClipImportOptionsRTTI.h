//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIType.h"
#include "Audio/BsAudioClipImportOptions.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT AudioClipImportOptionsRTTI : public TRTTIType<AudioClipImportOptions, ImportOptions, AudioClipImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Format, 0)
			B3D_RTTI_MEMBER(ReadMode, 1)
			B3D_RTTI_MEMBER(Is3D, 2)
			B3D_RTTI_MEMBER(BitDepth, 3)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "AudioClipImportOptions";
			return name;
		}

		u32 GetRttiId() const override
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
} // namespace b3d
