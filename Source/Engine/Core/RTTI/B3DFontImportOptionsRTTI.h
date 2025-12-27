//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStdRTTI.h"
#include "Text/B3DFontImportOptions.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(CharRange, TID_CharRange)

	class B3D_EXPORT FontImportOptionsRTTI : public TRTTIType<FontImportOptions, ImportOptions, FontImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(FontSizes, 0)
			B3D_RTTI_MEMBER(Dpi, 2)
			B3D_RTTI_MEMBER(RenderMode, 3)
			B3D_RTTI_MEMBER(Bold, 4)
			B3D_RTTI_MEMBER(Italic, 5)
			B3D_RTTI_MEMBER(CharIndexRanges, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "FontImportOptions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FontImportOptions;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FontImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
