//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DImportOptions.h"
#include "Text/B3DFont.h"

namespace b3d
{
	/** @addtogroup Text
	 *  @{
	 */

	/** Represents a range of character code. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Text), ExportAsStruct(true), API(Framework), API(Editor)) CharRange
	{
		CharRange() = default;

		CharRange(u32 start, u32 end)
			: Start(start), End(end)
		{}

		u32 Start = 0;
		u32 End = 0;
	};

	/**	Import options that allow you to control how is a font imported. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Text), API(Framework), API(Editor)) FontImportOptions : public ImportOptions
	{
	public:
		FontImportOptions() = default;

		/**	Determines font sizes that are to be imported. Sizes are in points. */
		B3D_SCRIPT_EXPORT()
		Vector<float> FontSizes = { };

		/**	Determines character index ranges to import. Ranges are defined as unicode numbers. */
		B3D_SCRIPT_EXPORT()
		Vector<CharRange> CharIndexRanges = { CharRange(33, 166) }; // Most used ASCII characters

		/**	Determines dots per inch scale that will be used when rendering the characters. */
		B3D_SCRIPT_EXPORT()
		u32 Dpi = 96;

		/**	Determines the render mode used for rendering the characters into a bitmap. */
		B3D_SCRIPT_EXPORT()
		FontRenderMode RenderMode = FontRenderMode::HintedSmooth;

		/**	Determines whether the bold font style should be used when rendering. */
		B3D_SCRIPT_EXPORT()
		bool Bold = false;

		/**	Determines whether the italic font style should be used when rendering. */
		B3D_SCRIPT_EXPORT()
		bool Italic = false;

		/** Creates a new import options object that allows you to customize how are fonts imported. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static TShared<FontImportOptions> Create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontImportOptionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
