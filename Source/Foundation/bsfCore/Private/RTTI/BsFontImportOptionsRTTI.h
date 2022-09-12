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
			BS_RTTI_MEMBER_PLAIN(fontSizes, 0)
			BS_RTTI_MEMBER_PLAIN(dpi, 2)
			BS_RTTI_MEMBER_PLAIN(renderMode, 3)
			BS_RTTI_MEMBER_PLAIN(bold, 4)
			BS_RTTI_MEMBER_PLAIN(italic, 5)
			BS_RTTI_MEMBER_PLAIN(charIndexRanges, 6)
		BS_END_RTTI_MEMBERS

		// For compability with old version
		Vector<std::pair<UINT32, UINT32>>& GetCharIndexRangesOld(FontImportOptions* obj)
		{
			static Vector<std::pair<UINT32, UINT32>> dummy;
			return dummy;
		}

		void SetCharIndexRangesOld(FontImportOptions* obj, Vector<std::pair<UINT32, UINT32>>& value)
		{
			// If already set it's assumed the new version already populated it
			if(!obj->charIndexRanges.Empty())
				return;

			for(auto& entry : value)
				obj->charIndexRanges.push_back(CharRange(entry.first, entry.second));
		}

	public:
		FontImportOptionsRTTI()
		{
			addPlainField("mCharIndexRangesOld", 1, &FontImportOptionsRTTI::getCharIndexRangesOld,
				&FontImportOptionsRTTI::setCharIndexRangesOld);
		}

		const String& GetRTTIName() override
		{
			static String name = "FontImportOptions";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_FontImportOptions;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<FontImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
}
