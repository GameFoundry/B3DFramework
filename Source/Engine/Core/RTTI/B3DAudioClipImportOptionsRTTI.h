//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Reflection/B3DRTTIType.h"
#include "Audio/B3DAudioClipImportOptions.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT AudioClipImportOptionsRTTI : public TRTTIType<AudioClipImportOptions, ImportOptions, AudioClipImportOptionsRTTI>
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

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<AudioClipImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
