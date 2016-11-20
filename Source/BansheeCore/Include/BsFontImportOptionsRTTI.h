//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsFontImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT FontImportOptionsRTTI : public RTTIType<FontImportOptions, ImportOptions, FontImportOptionsRTTI>
	{
	private:
		Vector<UINT32>& getFontSizes(FontImportOptions* obj) { return obj->mFontSizes; }
		void setFontSizes(FontImportOptions* obj, Vector<UINT32>& value) { obj->mFontSizes = value; }

		Vector<std::pair<UINT32, UINT32>>& getCharIndexRanges(FontImportOptions* obj) { return obj->mCharIndexRanges; }
		void setCharIndexRanges(FontImportOptions* obj, Vector<std::pair<UINT32, UINT32>>& value) { obj->mCharIndexRanges = value; }

		UINT32& getDPI(FontImportOptions* obj) { return obj->mDPI; }
		void setDPI(FontImportOptions* obj, UINT32& value) { obj->mDPI = value; }

		FontRenderMode& getRenderMode(FontImportOptions* obj) { return obj->mRenderMode; }
		void setRenderMode(FontImportOptions* obj, FontRenderMode& value) { obj->mRenderMode = value; }

		bool& getBold(FontImportOptions* obj) { return obj->mBold; }
		void setBold(FontImportOptions* obj, bool& value) { obj->mBold = value; }

		bool& getItalic(FontImportOptions* obj) { return obj->mItalic; }
		void setItalic(FontImportOptions* obj, bool& value) { obj->mItalic = value; }

	public:
		FontImportOptionsRTTI()
		{
			addPlainField("mFontSizes", 0, &FontImportOptionsRTTI::getFontSizes, &FontImportOptionsRTTI::setFontSizes);
			addPlainField("mCharIndexRanges", 1, &FontImportOptionsRTTI::getCharIndexRanges, &FontImportOptionsRTTI::setCharIndexRanges);
			addPlainField("mDPI", 2, &FontImportOptionsRTTI::getDPI, &FontImportOptionsRTTI::setDPI);
			addPlainField("mRenderMode", 3, &FontImportOptionsRTTI::getRenderMode, &FontImportOptionsRTTI::setRenderMode);
			addPlainField("mBold", 4, &FontImportOptionsRTTI::getBold, &FontImportOptionsRTTI::setBold);
			addPlainField("mItalic", 5, &FontImportOptionsRTTI::getItalic, &FontImportOptionsRTTI::setItalic);
		}

		const String& getRTTIName() override
		{
			static String name = "FontImportOptions";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_FontImportOptions;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<FontImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
}