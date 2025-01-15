//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"

namespace bs
{
	struct GUIStyleSheetRules;

	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Helper class that performs various operations related to GUI. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), Static) GUIUtility
	{
	public:
		/**
		 * Calculates the size of elements in a layout of the specified size.
		 *
		 * @param[in]	width				Width of the layout.
		 * @param[in]	height				Height of the layout.
		 * @param[in]	layout				Parent layout of the children to calculate the area for.
		 * @param[in]	updateOptimalSizes	Optimization (doesn't change the results). Set to false if
		 *									GUIElement::UpdateOptimalLayoutSizes was already called and optimal sizes
		 *									are up to date to avoid recalculating them. (Normally that is true if this is
		 *									being called during GUI layout update)
		 */
		static Vector2I CalcActualSize(u32 width, u32 height, GUILayout* layout, bool updateOptimalSizes = true);

		/**
		 * Calculates size of the GUI element area based on the GUI content size. This is just the content area expanded by padding and border provided by the style.
		 *
		 * @param	contentSize		Size of the GUI element's content area.
		 * @param	styleSheetRule	Style to use when rendering the GUI element.
		 * @return					Size of the GUI element (including the border).
		 */
		static Size2UI CalculateSizeWithPaddingAndBorder(const Size2UI& contentSize, const GUIStyleSheetRules& styleSheetRule);

		/**
		 * Calculates optimal size for displaying particular GUI contents.
		 *
		 * @param	content			Content to calculate size for.
		 * @param	styleSheetRule	Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line. Only relevant
		 *							if the contents contain text.
		 * @return					Optimal size of the GUI element, including content size, style padding and border width.
		 */
		static Size2UI CalculateOptimalContentSizeWithPaddingAndBorder(const GUIContent& content, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth = 0);

		/**
		 * Calculates optimal size for displaying text.
		 *
		 * @param	text			Text to calculate size for.
		 * @param	styleSheetRule	Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line.
		 * @return					Optimal size of the GUI element, including content size and the style padding (area within GUI elements border).
		 */
		static Size2UI CalculateOptimalContentSizeWithPaddingAndBorder(const String& text, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth = 0);

		/**
		 * Calculates the content area based on the total element size (i.e. size as calculated by the layout). This is the layout area potentially
		 * offset/reduced by border/padding as specified in the style sheet rules.
		 */
		static Rect2I CalculateContentArea(const Size2UI& layoutSize, const GUIStyleSheetRules& styleSheetRules);

		/**
		 * Calculates optimal content size for the provided text using the provided font and size. Size is calculated
		 * without word wrap.
		 *
		 * @param	text			Text to calculate the size for.
		 * @param	font			Font to use for rendering the text.
		 * @param	fontSize		Size of individual characters in the font, in points.
		 * @return					Width/height required to display the text, in pixels.
		 */
		B3D_SCRIPT_EXPORT()
		static Vector2I CalculateTextBounds(const String& text, const HFont& font, float fontSize);
	private:
		/**
		 * Calculates the size of elements in a layout of the specified size. Assumes the layout and all its children
		 * have updated optimal sizes.
		 *
		 * @param[in]	width				Width of the layout.
		 * @param[in]	height				Height of the layout.
		 * @param[in]	layout				Parent layout of the children to calculate the area for.
		 */
		static Vector2I CalcActualSizeInternal(u32 width, u32 height, GUILayout* layout);
	};

	/** @} */
} // namespace bs
