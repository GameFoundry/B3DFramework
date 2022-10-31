//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Text/BsFontImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	BS_ALLOW_MEMCPY_SERIALIZATION(CharRange)

	class BS_CORE_EXPORT FontImportOptionsRTTI : public RTTIType<FontImportOptions, ImportOptions, FontImportOptionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(FontSizes, 0)
			BS_RTTI_MEMBER_PLAIN(Dpi, 2)
			BS_RTTI_MEMBER_PLAIN(RenderMode, 3)
			BS_RTTI_MEMBER_PLAIN(Bold, 4)
			BS_RTTI_MEMBER_PLAIN(Italic, 5)
			BS_RTTI_MEMBER_PLAIN(CharIndexRanges, 6)
		BS_END_RTTI_MEMBERS

		// For compability with old version
		Vector<std::pair<u32, u32>>& GetCharIndexRangesOld(FontImportOptions* obj)
		{
			static Vector<std::pair<u32, u32>> dummy;
			return dummy;
		}

		void SetCharIndexRangesOld(FontImportOptions* obj, Vector<std::pair<u32, u32>>& value)
		{
			// If already set it's assumed the new version already populated it
			if(!obj->CharIndexRanges.empty())
				return;

			for(auto& entry : value)
				obj->CharIndexRanges.push_back(CharRange(entry.first, entry.second));
		}

	public:
		FontImportOptionsRTTI()
		{
			AddPlainField("mCharIndexRangesOld", 1, &FontImportOptionsRTTI::GetCharIndexRangesOld, &FontImportOptionsRTTI::SetCharIndexRangesOld);
		}

		const String& GetRttiName()
		{
			static String name = "FontImportOptions";
			return name;
		}

		u32 GetRttiId()
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
} // namespace bs
