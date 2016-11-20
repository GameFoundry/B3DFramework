//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsImportOptions.h"
#include "BsFont.h"

namespace bs
{
	/** @addtogroup Text
	 *  @{
	 */

	/**	Determines how is a font rendered into the bitmap texture. */
	enum class FontRenderMode
	{
		Smooth, /*< Render antialiased fonts without hinting (slightly more blurry). */
		Raster, /*< Render non-antialiased fonts without hinting (slightly more blurry). */
		HintedSmooth, /*< Render antialiased fonts with hinting. */
		HintedRaster /*< Render non-antialiased fonts with hinting. */
	};

	/**	Import options that allow you to control how is a font imported. */
	class BS_CORE_EXPORT FontImportOptions : public ImportOptions
	{
	public:
		FontImportOptions();

		/**	Sets font sizes that are to be imported. Sizes are in points. */
		void setFontSizes(const Vector<UINT32>& fontSizes) { mFontSizes = fontSizes; }

		/**	Adds an index range of characters to import.  */
		void addCharIndexRange(UINT32 from, UINT32 to);

		/**	Clears all character indexes, so no character are imported. */
		void clearCharIndexRanges();

		/**	Sets dots per inch resolution to use when rendering the characters into the texture. */
		void setDPI(UINT32 dpi) { mDPI = dpi; }

		/**	Set the render mode used for rendering the characters into a bitmap. */
		void setRenderMode(FontRenderMode renderMode) { mRenderMode = renderMode; }

		/**	Sets whether the bold font style should be used when rendering. */
		void setBold(bool bold) { mBold = bold; }

		/**	Sets whether the italic font style should be used when rendering. */
		void setItalic(bool italic) { mItalic = italic; }

		/**	Gets the sizes that are to be imported. Ranges are defined as unicode numbers. */
		Vector<UINT32> getFontSizes() const { return mFontSizes; }

		/**	Gets character index ranges to import. Ranges are defined as unicode numbers. */
		Vector<std::pair<UINT32, UINT32>> getCharIndexRanges() const { return mCharIndexRanges; }

		/**	Returns dots per inch scale that will be used when rendering the characters. */
		UINT32 getDPI() const { return mDPI; }

		/**	Get the render mode used for rendering the characters into a bitmap. */
		FontRenderMode getRenderMode() const { return mRenderMode; }

		/**	Sets whether the bold font style should be used when rendering. */
		bool getBold() const { return mBold; }

		/**	Sets whether the italic font style should be used when rendering. */
		bool getItalic() const { return mItalic; }

	private:
		Vector<UINT32> mFontSizes;
		Vector<std::pair<UINT32, UINT32>> mCharIndexRanges;
		UINT32 mDPI;
		FontRenderMode mRenderMode;
		bool mBold;
		bool mItalic;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}