//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGUIContent.h"
#include "BsPrerequisites.h"
#include "Image/BsColor.h"
#include "Utility/BsBitfield.h"
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

	enum class GUIStyleSheetPropertyType
	{
		Undefined,

		Width,
		Height,
		MinWidth,
		MinHeight,
		MaxWidth,
		MaxHeight,

		Margin,
		MarginTop,
		MarginBottom,
		MarginLeft,
		MarginRight,

		Padding,
		PaddingTop,
		PaddingBottom,
		PaddingLeft,
		PaddingRight,

		Color,
		Opacity,
		BackgroundColor,

		TextAlign,
		VerticalAlign,
		FontFamily,
		FontSize,

		Border,
		BorderStyle,
		BorderWidth,
		BorderColor,

		BorderTop,
		BorderTopStyle,
		BorderTopWidth,
		BorderTopColor,

		BorderBottom,
		BorderBottomStyle,
		BorderBottomWidth,
		BorderBottomColor,

		BorderLeft,
		BorderLeftStyle,
		BorderLeftWidth,
		BorderLeftColor,

		BorderRight,
		BorderRightStyle,
		BorderRightWidth,
		BorderRightColor,

		BorderRadius,
		BorderTopLeftRadius,
		BorderTopRightRadius,
		BorderBottomLeftRadius,
		BorderBottomRightRadius,

		Count,
	};

	struct GUIBorderElement
	{
		u32 Width = 0;
		Color Color;
		GUIBorderElementStyle Style = GUIBorderElementStyle::None;
	};

	struct GUIStyleSheetStateStyle
	{
		String Selector;
		String PseudoClass;
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

		u32 BorderTopLeftRadius = 0;
		u32 BorderTopRightRadius = 0;
		u32 BorderBottomLeftRadius = 0;
		u32 BorderBottomRightRadius = 0;

		String FontFamily;
		u32 FontSize = 8;
		GUIHorizontalTextAlignment HorizontalTextAlignment = GUIHorizontalTextAlignment::Left;
		GUIVerticalTextAlignment VerticalTextAlignment = GUIVerticalTextAlignment::Middle;

		static constexpr u32 kPropertyDWordCount = Math::DivideAndRoundUp((u32)GUIStyleSheetPropertyType::Count, (u32)sizeof(u32));
		TBitfield<InlineContainerAllocator<kPropertyDWordCount>> OverridenProperties;

		void Override(const GUIStyleSheetStateStyle& other);
	};

	struct GUIStyleSheetStyle
	{
		GUIStyleSheetStateStyle Normal;
		Optional<GUIStyleSheetStateStyle> Hover;
		Optional<GUIStyleSheetStateStyle> Active;
		Optional<GUIStyleSheetStateStyle> Focused;
		Optional<GUIStyleSheetStateStyle> Disabled;
		Optional<GUIStyleSheetStateStyle> Checked;
	};

	// TODO - Doc
	class B3D_EXPORT GUIStyleSheet
	{
	public:
		static Optional<GUIStyleSheet> Parse(const Path& file);

		GUIStyleSheetStateStyle FindStyle(const String& elementType, const String& elementId, GUIElementState state);

	private:
		UnorderedMap<String, GUIStyleSheetStyle> mElementStyles;
		UnorderedMap<String, GUIStyleSheetStyle> mIdStyles;
	};


	/** @} */
} // namespace bs
