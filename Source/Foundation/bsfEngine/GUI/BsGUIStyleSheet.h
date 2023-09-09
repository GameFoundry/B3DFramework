//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Image/BsColor.h"
#include "Utility/BsRectOffset.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	enum class GUIBorderElementStyle
	{
		None,
		Solid
	};

	enum class GUIStyleSheetSelectorType
	{
		Element,
		Id
	};

	enum class GUIHorizontalTextAlignment
	{
		Left, Center, Right
	};

	enum class GUIVerticalTextAlignment
	{
		Top, Middle, Bottom
	};

	struct GUIBorderElement
	{
		u32 Width = 0;
		Color Color;
		GUIBorderElementStyle Style = GUIBorderElementStyle::None;
	};

	struct GUIStyleSheetElement
	{
		String Selector;
		String State;
		GUIStyleSheetSelectorType SelectorType = GUIStyleSheetSelectorType::Element;

		RectOffset Margins;
		RectOffset Padding;

		Size2UI Size;
		Size2UI MinimumSize;
		Size2UI MaximumSize;

		Color BackgroundColor;
		Color Color;
		float Opacity = 1.0f;

		GUIBorderElement BorderLeft;
		GUIBorderElement BorderRight;
		GUIBorderElement BorderTop;
		GUIBorderElement BorderBottom;

		float BorderLeftRadius = 0.0f;
		float BorderRightRadius = 0.0f;
		float BorderTopRadius = 0.0f;
		float BorderBottomRadius = 0.0f;

		String FontFamily;
		float FontSize = 8.0f;
		GUIHorizontalTextAlignment HorizontalTextAlignment = GUIHorizontalTextAlignment::Left;
		GUIVerticalTextAlignment VerticalTextAlignment = GUIVerticalTextAlignment::Middle;
	};

	// TODO - Doc
	class GUIStyleSheet
	{
		GUIStyleSheet Parse(const Path& file);
	};


	/** @} */
} // namespace bs
