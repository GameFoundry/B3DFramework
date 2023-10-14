//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUIContent.h"

namespace bs
{
	struct GUIStyleSheetStateRule;
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Contains various helper methods used by GUI. */
	class B3D_EXPORT GUIHelper
	{
	public:
		/**
		 * Calculates optimal content size by returning the nearest valid size to the provided value.
		 *
		 * @param	contentSize		Wanted content size. This will be limited by minimal constraints of the style and layout options.
		 * @param	style			Style to use for determining size constraints.
		 * @param	dimensions		Dimension constraints of a GUI element.
		 */
		static Vector2I CalculateOptimalContentSize(const Vector2I& contentSize, const GUIElementStyle& style, const GUIDimensions& dimensions);

		/**
		 * Calculates optimal content size for the provided content using the provided style and layout options for
		 * constraints.
		 *
		 * @param	content			Content to calculate size for.
		 * @param	style			Style to use for determining size constraints.
		 * @param	dimensions		Dimension constraints of a GUI element.
		 * @param	state			State of the GUI element in case the content changes according to state.
		 */
		static Vector2I CalculateOptimalContentSize(const GUIContent& content, const GUIElementStyle& style, const GUIDimensions& dimensions, GUIElementState state = GUIElementState::Normal);

		/**
		 * Calculates optimal content size for the provided text using the provided style and layout options for
		 * constraints.
		 *
		 * @param	text			Text to calculate size for.
		 * @param	style			Style to use for determining size constraints.
		 * @param	dimensions		Dimension constraints of a GUI element.
		 */
		static Vector2I CalculateOptimalContentSize(const String& text, const GUIElementStyle& style, const GUIDimensions& dimensions);

		/**
		 * Calculates size of the GUI element area based on the GUI content size. This is just the content area expanded by padding and border provided by the style.
		 *
		 * @param	contentSize		Size of the GUI element's content area.
		 * @param	style			Style to use when rendering the GUI element.
		 * @return					Size of the GUI element (including the border).
		 */
		static Size2UI CalculateSizeWithPaddingAndBorder(const Size2UI& contentSize, const GUIStyleSheetStateRule& style);

		/**
		 * Calculates optimal size for displaying particular GUI contents.
		 *
		 * @param	content			Content to calculate size for.
		 * @param	style			Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line. Only relevant
		 *							if the contents contain text.
		 * @return					Optimal size of the GUI element, including content size, style padding and border width.
		 */
		static Size2UI CalculateOptimalContentSizeWithPaddingAndBorder(const GUIContent& content, const GUIStyleSheetStateRule& style, u32 wordWrapWidth = 0);

		/**
		 * Calculates optimal size for displaying text.
		 *
		 * @param	text			Text to calculate size for.
		 * @param	style			Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line.
		 * @return					Optimal size of the GUI element, including content size and the style padding (area within GUI elements border).
		 */
		static Size2UI CalculateOptimalContentSizeWithPaddingAndBorder(const String& text, const GUIStyleSheetStateRule& style, u32 wordWrapWidth = 0);

		/**
		 * Calculates optimal content size for the provided text using the provided font and size. Size is calculated
		 * without word wrap.
		 *
		 * @param	text			Text to calculate the size for.
		 * @param	font			Font to use for rendering the text.
		 * @param	fontSize		Size of individual characters in the font, in points.
		 * @return					Width/height required to display the text, in pixels.
		 */
		static Vector2I CalculateTextBounds(const String& text, const HFont& font, u32 fontSize);
	};

	/** @} */
} // namespace bs
