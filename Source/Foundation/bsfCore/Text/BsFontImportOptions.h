//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Importer/BsImportOptions.h"
#include "Text/BsFont.h"

namespace bs
{
	/** @addtogroup Text
	 *  @{
	 */

	/**	Determines how is a font rendered into the bitmap texture. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Text),api:bsf,api:bed) FontRenderMode
	{
		Smooth, /*< Render antialiased fonts without hinting (slightly more blurry). */
		Raster, /*< Render non-antialiased fonts without hinting (slightly more blurry). */
		HintedSmooth, /*< Render antialiased fonts with hinting. */
		HintedRaster /*< Render non-antialiased fonts with hinting. */
	};

	/** Represents a range of character code. */
	struct BS_SCRIPT_EXPORT(DocumentationGroup(Text),ExportAsStruct(true),api:bsf,api:bed) CharRange
	{
		CharRange() = default;
		CharRange(u32 start, u32 end)
			: Start(start), End(end)
		{ }

		u32 Start = 0;
		u32 End = 0;
	};

	/**	Import options that allow you to control how is a font imported. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Text),api:bsf,api:bed) FontImportOptions : public ImportOptions
	{
	public:
		FontImportOptions() = default;

		/**	Determines font sizes that are to be imported. Sizes are in points. */
		BS_SCRIPT_EXPORT()
		Vector<u32> FontSizes = { 10 };

		/**	Determines character index ranges to import. Ranges are defined as unicode numbers. */
		BS_SCRIPT_EXPORT()
		Vector<CharRange> CharIndexRanges = { CharRange(33, 166) }; // Most used ASCII characters

		/**	Determines dots per inch scale that will be used when rendering the characters. */
		BS_SCRIPT_EXPORT()
		u32 Dpi = 96;

		/**	Determines the render mode used for rendering the characters into a bitmap. */
		BS_SCRIPT_EXPORT()
		FontRenderMode RenderMode = FontRenderMode::HintedSmooth;

		/**	Determines whether the bold font style should be used when rendering. */
		BS_SCRIPT_EXPORT()
		bool Bold = false;

		/**	Determines whether the italic font style should be used when rendering. */
		BS_SCRIPT_EXPORT()
		bool Italic = false;

		/** Creates a new import options object that allows you to customize how are fonts imported. */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<FontImportOptions> Create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontImportOptionsRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
}
